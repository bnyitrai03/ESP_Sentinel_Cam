#include "qr_reader_app.h"
#include "error_handler.h"
#include "esp_log.h"

constexpr auto *TAG = "QR Reader app";

std::atomic<bool> QRReaderApp::_shutdown_requested{false};
std::atomic<bool> QRReaderApp::_qr_code_decoded{false};

QRReaderApp::QRReaderApp() : _cam(true) { _cam.start(); }

// ***************************   Main logic   *************************** //
void QRReaderApp::run() {
  // -------- Get the WiFi and server information from the QR code -------- //
  get_qr_code();

  // -------- Connect to the WiFi network -------- //
  _wifi.connect();
}
// ********************************************************************** //

void QRReaderApp::qr_decode_task(void *arg) {
  auto *context = static_cast<TaskContext *>(arg);

  while (!_shutdown_requested && !context->decoded) {
    camera_fb_t *pic = NULL;
    if (xQueueReceive(context->queue, &pic, pdMS_TO_TICKS(10000)) != pdTRUE) {
      ESP_LOGE(TAG, "Failed to receive frame from queue");
      continue;
    }

    if (context->decoder.decode_frame(pic)) {
      context->decoded = true;
    }
  }

  delete context;
  vTaskDelete(nullptr);
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
      5, NULL, tskNO_AFFINITY);
  if (result != pdPASS) {
    ESP_LOGE(TAG, "Failed to create task");
    restart();
  }

  /*
   * The loop is responsible for getting the camera frame buffer and
   * sending it to the QR code decoding task. The loop runs until the QR code
   * is decoded or a shutdown is requested.
   */
  while (!_qr_code_decoded && !_shutdown_requested) {
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
  vQueueDelete(processing_queue);
  esp_camera_deinit();
  ESP_LOGI(TAG, "QR code decoded!");
}