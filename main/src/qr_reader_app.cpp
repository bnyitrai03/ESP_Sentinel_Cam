#include "qr_reader_app.h"
#include "error_handler.h"
#include "esp_log.h"
#include "event_manager.h"
#include "http_client.h"
#include "led.h"
#include "storage.h"
#include <ArduinoJson.h>

constexpr auto *TAG = "QR Reader app";

std::atomic<bool> QRReaderApp::_qr_code_decoded{false};

QRReaderApp::QRReaderApp() : _cam(true) { _cam.start(); }

void QRReaderApp::start() {
  xTaskCreate(qr_task, "qr_app_task", 8192, this, 5, &_qr_app_task_handle);
}

void QRReaderApp::stop() {

  if (_decode_task_handle != nullptr) {
    eTaskState taskState = eTaskGetState(_decode_task_handle);
    if (taskState != eDeleted && taskState != eInvalid) {
      vTaskSuspend(_decode_task_handle);
      vTaskDelete(_decode_task_handle);
      _decode_task_handle = nullptr;
    }
  }
  if (_qr_app_task_handle != nullptr) {
    eTaskState taskState = eTaskGetState(_qr_app_task_handle);
    if (taskState != eDeleted && taskState != eInvalid) {
      vTaskSuspend(_qr_app_task_handle);
      vTaskDelete(_qr_app_task_handle);
      ESP_LOGI(TAG, "Stopped QR reader task");
      _qr_app_task_handle = nullptr;
    }
  }
}

void QRReaderApp::qr_task(void *pvParameters) {
  QRReaderApp *app = static_cast<QRReaderApp *>(pvParameters);

  app->run();
}

// ***************************   Main logic   *************************** //
void QRReaderApp::run() {
  // -------- Get the WiFi and server information from the QR code -------- //
  get_qr_code();

  // -------- Connect to the WiFi network -------- //
  _wifi.connect();

  // -------- Get the static configuration from the server -------- //
  get_static_config();

  // Signaling the correct execution of the app
  Led::set_pattern(Led::Pattern::STATIC_CONFIG_SAVED_BLINK);
  vTaskDelay(pdMS_TO_TICKS(3000));

  // -------- Restart the device to start the camera app -------- //
  PUBLISH(EventType::STOP_BUTTON);
  deinit_components();
  Storage::write("mode", "cam"); // change app mode
  ESP_LOGI(TAG, "Restarting the device to start the camera app");
  esp_restart();
}
// ********************************************************************** //

void QRReaderApp::qr_decode_task(void *arg) {
  auto *context = static_cast<TaskContext *>(arg);

  while (!context->decoded) {
    camera_fb_t *pic = NULL;
    if (xQueueReceive(context->queue, &pic, pdMS_TO_TICKS(10000)) != pdTRUE) {
      ESP_LOGE(TAG, "Failed to receive frame from queue");
      continue;
    }

    if (context->decoder.decode_frame(pic)) {
      context->decoded = true;
    }
    vTaskDelay(pdMS_TO_TICKS(50));
  }

  delete context;
  // The qr task handles the clean up
  while (true) {
    vTaskDelay(portMAX_DELAY);
  }
}

void QRReaderApp::get_qr_code() {
  // The pictures are sent through the queue to the QR code decoding task
  auto processing_queue = xQueueCreate(1, sizeof(camera_fb_t *));
  if (!processing_queue) {
    ESP_LOGE(TAG, "Failed to create queue");
    restart();
  }

  // Camera framebuffer is: FRAMESIZE_VGA = 640x480
  auto context = std::make_unique<TaskContext>(640, 480, processing_queue,
                                               _qr_code_decoded);

  ESP_LOGI(TAG, "Starting QR code decoding task");

  auto result = xTaskCreatePinnedToCore(
      &qr_decode_task, "qr_decode", 24000,
      context.release(), // Transfer ownership to the task
      5, &_decode_task_handle, tskNO_AFFINITY);
  if (result != pdPASS) {
    ESP_LOGE(TAG, "Failed to create task");
    restart();
  }

  /*
   * The loop is responsible for getting the camera frame buffer and
   * sending it to the QR code decoding task. The loop runs until the QR code
   * is decoded.
   */
  while (!_qr_code_decoded) {
    camera_fb_t *pic = NULL;
    pic = esp_camera_fb_get();
    if (pic) {
      xQueueSend(processing_queue, &pic, pdMS_TO_TICKS(5000));
    } else {
      ESP_LOGE(TAG, "Failed to get frame buffer");
    }
    esp_camera_fb_return(pic);
    vTaskDelay(pdMS_TO_TICKS(500));
  }

  // Clean up
  ESP_LOGI(TAG, "QR code decoded!");
  if (_decode_task_handle != nullptr) {
    vTaskDelete(_decode_task_handle);
    _decode_task_handle = nullptr;
  }
  Led::set_pattern(Led::Pattern::ON);
  vQueueDelete(processing_queue);
  esp_camera_deinit();
}

void QRReaderApp::get_static_config() {
  JsonDocument response;
  char server_url[256];
  Storage::read("server_url", server_url, sizeof(server_url));
  const int MAX_RETRIES = 5;
  const int RETRY_DELAY_MS = 5000;

  for (int retry = 0; retry < MAX_RETRIES; retry++) {
    // Send a GET request to the server to get the static configuration
    esp_err_t result = HTTPClient::get_config(server_url, response);

    switch (result) {
    case ESP_OK:
      save_static_config(response);
      return;

    case ESP_FAIL:
      if (retry < MAX_RETRIES - 1) {
        ESP_LOGW(TAG,
                 "Failed to get static configuration (attempt %d/%d). Retrying "
                 "in %d ms...",
                 retry + 1, MAX_RETRIES, RETRY_DELAY_MS);
        vTaskDelay(pdMS_TO_TICKS(RETRY_DELAY_MS));
        continue;
      }
      ESP_LOGE(TAG, "Failed to get static configuration after %d attempts",
               MAX_RETRIES);
      restart();
      break;

    case ESP_ERR_NOT_FOUND:
      restart();
      break;

    default:
      ESP_LOGE(TAG, "Unknown error");
      restart();
      break;
    }
  }
}

void QRReaderApp::save_static_config(const JsonDocument &doc) {
  Storage::write("mqttAddress", doc["mqttAddress"].as<std::string>());
  Storage::write("mqttUser", doc["mqttUser"].as<std::string>());
  Storage::write("mqttPassword", doc["mqttPassword"].as<std::string>());
  Storage::write("imageTopic", doc["imageTopic"].as<std::string>());
  Storage::write("imageAckTopic", doc["imageAckTopic"].as<std::string>());
  Storage::write("healthRepTopic", doc["healthReportTopic"].as<std::string>());
  Storage::write("configTopic", doc["healthReportRespTopic"].as<std::string>());
  Storage::write("logTopic", doc["logTopic"].as<std::string>());
  Storage::write("cameraMode", doc["cameraMode"].as<std::string>());
  ESP_LOGI(TAG, "Static configuration saved!");
}