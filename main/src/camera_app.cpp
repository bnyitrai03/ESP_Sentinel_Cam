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
  auto res = xTaskCreate(camera_task, "camera_task", 8192, this, 5,
                         &_camera_task_handle);
  if (res != pdPASS) {
    ESP_LOGE(TAG, "Failed to create camera task");
    restart();
  }
}

void CameraApp::stop() {
  // signal to the camera task that it was stopped from another task
  _stopped_from_other_task = true;
  if (_camera_task_handle != nullptr) {
    eTaskState taskState = eTaskGetState(_camera_task_handle);
    if (taskState != eDeleted && taskState != eInvalid) {
      vTaskSuspend(_camera_task_handle);
      vTaskDelete(_camera_task_handle);
      _camera_task_handle = nullptr;
      ESP_LOGI(TAG, "Stopped camera task");
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
    ESP_LOGI(TAG, "Camera task finished");
    PUBLISH(EventType::SLEEP_UNTIL_NEXT_PERIOD);
  }
  while (1) {
    vTaskDelay(portMAX_DELAY);
  }
}

// ***************************   Main logic   *************************** //

void CameraApp::run() {
  if (!initialize()) {
    return;
  }

  if (!handle_config_update()) {
    return;
  }

  if (!capture_and_send_image()) {
    return;
  }
}
// ********************************************************************* //

// ***************************   Helper functions   ******************** //

bool CameraApp::initialize() {
  _wifi.connect();
  _wifi.sync_time();
  _mqtt.start();
  _sensors.init();
  _config.load_from_storage();
  Led::set_pattern(Led::Pattern::MQTT_CONNECTED_BLINK);

  if (_config.set_active_config() == -1) {
    ESP_LOGE(TAG, "Failed to set active configuration");
    return false;
  }

  return true;
}

bool CameraApp::handle_config_update() {
  if (send_health_report() != ESP_OK) {
    ESP_LOGE(TAG, "Failed to publish health report!");
    return false;
  }

  if (!_mqtt.wait_for_config(calculate_max_wait())) {
    ESP_LOGE(TAG, "Failed to receive new config or config-ok!");
    return false;
  }

  // Process new configuration if received
  if (_mqtt.get_new_config_received()) {
    vTaskDelay(100 / portTICK_RATE_MS);
    if (_config.set_active_config() == -1) {
      return false;
    }
  }

  return true;
}

bool CameraApp::capture_and_send_image() {
  _cam.start();
  _cam.take_image();
  char timestamp[TIMESTAMP_SIZE] = {0};
  Time::get_date(timestamp, sizeof(timestamp));

  if (send_image_header(timestamp) != ESP_OK) {
    ESP_LOGE(TAG, "Failed to publish image header!");
    return false;
  }

  if (!_mqtt.wait_for_header_ack(timestamp, calculate_max_wait())) {
    ESP_LOGE(TAG, "No matching timestamp received, skipping image publish!");
    return false;
  }

  if (send_image() != ESP_OK) {
    ESP_LOGE(TAG, "Failed to publish image!");
    return false;
  }

  return true;
}

esp_err_t CameraApp::send_health_report() {
  JsonDocument doc;
  char timestamp[TIMESTAMP_SIZE] = {0};
  Time::get_date(timestamp, sizeof(timestamp));

  // create health report json
  doc["timestamp"] = timestamp;
  doc["configId"] = _config.get_uuid();
  doc["period"] = _config.get_period();
  _sensors.read_sensors(doc);

  return send_json(doc, _mqtt.get_health_report_topic());
}

esp_err_t CameraApp::send_image_header(const char *timestamp) {
  JsonDocument doc;

  // create image header json
  doc["timestamp"] = timestamp;
  doc["size"] = _cam.get_image_size();
  doc["mode"] = _cam.get_camera_mode();
  doc["width"] = _cam.get_width();
  doc["height"] = _cam.get_height();

  return send_json(doc, _mqtt.get_image_topic());
}

esp_err_t CameraApp::send_json(JsonDocument &doc, const char *topic) {
  std::string json;
  serializeJson(doc, json);
  return _mqtt.publish(topic, json.c_str(), json.size());
}

esp_err_t CameraApp::send_image() {
  if (_mqtt.publish(_mqtt.get_image_topic(), _cam.get_image_data(),
                    _cam.get_image_size()) != ESP_OK) {
    ESP_LOGE(TAG, "Failed to publish image!");
    return ESP_FAIL;
  } else {
    ESP_LOGI(TAG, "Image published!");
    return ESP_OK;
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
// ************************************************************************** //