#include "camera_app.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "mysleep.h"
#include "mytime.h"
#include <ArduinoJson.h>
#include <esp_log.h>
#include <esp_system.h>
#include <sys/param.h>

#ifndef portTICK_RATE_MS
#define portTICK_RATE_MS portTICK_PERIOD_MS
#endif

constexpr auto *TAG = "Camera app";

CameraApp::CameraApp() : _cam(false) {}

void CameraApp::start() {
  xTaskCreate(camera_task, "camera_task", 8192, this, 5, &_camera_task_handle);
}

void CameraApp::stop() {
  _stopped_from_other_task = true;
  ESP_LOGI(TAG, "Attempting to stop camera task");
  if (_camera_task_handle != nullptr) {
    eTaskState taskState = eTaskGetState(_camera_task_handle);
    if (taskState != eDeleted && taskState != eInvalid) {
      ESP_LOGI(TAG, "Camera task in state: %d, suspending...", taskState);
      vTaskSuspend(_camera_task_handle);
      ESP_LOGI(TAG, "Camera task suspended, deleting...");
      vTaskDelete(_camera_task_handle);
      ESP_LOGI(TAG, "Stopped camera task");
      _camera_task_handle = nullptr;
    } else {
      ESP_LOGW(TAG, "Camera task in invalid state: %d", taskState);
    }
  } else {
    ESP_LOGW(TAG, "Camera task handle is null");
  }
}

void CameraApp::camera_task(void *pvParameters) {
  CameraApp *app = static_cast<CameraApp *>(pvParameters);

  app->run();

  if (app->_stopped_from_other_task == false) {
    PUBLISH(EventType::SLEEP_UNTIL_NEXT_PERIOD);
  }
  ESP_LOGI(TAG, "Camera task suspended");
  while (1) {
    vTaskDelay(portMAX_DELAY);
  }
}

// ***************************   Main logic   *************************** //
void CameraApp::run() {
  // ---------------------- Init ------------------- //
  _wifi.connect();
  _wifi.sync_time();
  // Disable lib logging else remote logging dies
  esp_log_level_set("mqtt5_client", ESP_LOG_NONE);
  esp_log_level_set("mqtt_client", ESP_LOG_NONE);
  _mqtt.start();
  _sensors.init();
  _config.load_from_storage();
  Led::set_pattern(Led::Pattern::MQTT_CONNECTED_BLINK);
  if (_config.set_active_config() == -1) {
    return;
  }

  multi_heap_info_t info;
  heap_caps_get_info(&info, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  ESP_LOGI(TAG, "total currently free in all non-continues blocks: %d",
           info.total_free_bytes);
  ESP_LOGI(TAG, "minimum free ever: %d", info.minimum_free_bytes);
  ESP_LOGI(TAG, "largest continues block: %d", info.largest_free_block);

  // -------- Send health report and check for new config -------- //
  if (send_health_report() != ESP_OK) {
    ESP_LOGE(TAG, "Failed to publish health report!");
  } else {
    ESP_LOGI(TAG, "JSON health report sent!");
  }
  if (!_mqtt.wait_for_config(calculate_max_wait())) {
    ESP_LOGE(TAG, "Failed to receive new config or config-ok!");
    return;
  }
  // if a new config is received, set it as active
  if (_mqtt.get_new_config_received()) {
    vTaskDelay(500 / portTICK_RATE_MS);
    if (_config.set_active_config() == -1) {
      return;
    }
  }

  // ------------ Create image and send the header ------------ //
  _cam.start();
  _cam.take_image();
  // create timestamp
  char timestamp[TIMESTAMP_SIZE] = {0};
  Time::get_date(timestamp, sizeof(timestamp));
  if (send_image_header(timestamp) != ESP_OK) {
    ESP_LOGE(TAG, "Failed to publish image header!");
  }

  // ------------------------ Send image ----------------------- //
  if (_mqtt.wait_for_header_ack(timestamp, calculate_max_wait())) {
    // Timestamp matches, proceed with sending image
    send_image();
  } else {
    ESP_LOGE(TAG, "No matching timestamp received, skipping image publish!");
  }

  // ---------------------- Deinit ------------------- //
  _cam.return_fb();
}
// ********************************************************************* //

esp_err_t CameraApp::send_health_report() {
  JsonDocument doc;
  // create timestamp
  char timestamp[TIMESTAMP_SIZE] = {0};
  Time::get_date(timestamp, sizeof(timestamp));
  // create health report json
  doc["timestamp"] = timestamp;
  doc["configId"] = _config.get_uuid();
  doc["period"] = _config.get_period();
  if (_sensors.read_sensors(doc) != ESP_OK) {
    ESP_LOGE(TAG, "Failed to read sensors!");
  }
  return send_json(doc, _mqtt.get_health_report_topic());
}

esp_err_t CameraApp::send_image_header(const char *timestamp) {
  JsonDocument doc;
  // create image header json
  doc["timestamp"] = timestamp;
  doc["size"] = _cam.get_image_size();
  doc["mode"] = "GRAY"; // need to implement color mode
  doc["width"] = _cam.get_width();
  doc["height"] = _cam.get_height();
  return send_json(doc, _mqtt.get_image_topic());
}

esp_err_t CameraApp::send_json(JsonDocument &doc, const char *topic) {
  std::string json;
  serializeJson(doc, json);
  return _mqtt.publish(topic, json.c_str(), json.size());
}

void CameraApp::send_image() {
  if (_mqtt.publish(_mqtt.get_image_topic(), _cam.get_image_data(),
                    _cam.get_image_size()) != ESP_OK) {
    ESP_LOGE(TAG, "Failed to publish image!");
  } else {
    ESP_LOGI(TAG, "Image published!");
  }
}

uint32_t CameraApp::calculate_max_wait() {
  int32_t elapsed_time = static_cast<int32_t>(esp_timer_get_time()) / 1000;
  int32_t max_wait = static_cast<int32_t>(_config.get_period() * 1000) -
                     elapsed_time - static_cast<int32_t>(OVERHEAD) / 1000;
  max_wait = MAX(max_wait, 0);
  ESP_LOGI(TAG, "Max wait time: %lu ms", max_wait);
  return static_cast<uint32_t>(max_wait);
}