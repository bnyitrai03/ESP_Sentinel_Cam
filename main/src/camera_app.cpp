#include "camera_app.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mysleep.h"
#include "mytime.h"
#include <ArduinoJson.h>
#include <esp_log.h>
#include <esp_system.h>
#include <sys/param.h>

#ifndef portTICK_RATE_MS
#define portTICK_RATE_MS portTICK_PERIOD_MS
#endif

constexpr auto *TAG = "camera_app";

std::atomic<bool> CameraApp::shutdown_requested = false;

CameraApp::CameraApp() {
  _cam.start();
  _wifi.connect();
  _config.load_from_storage();
  // Disable lib logging else remote logging dies
  esp_log_level_set("mqtt5_client", ESP_LOG_NONE);
  esp_log_level_set("mqtt_client", ESP_LOG_NONE);
  _mqtt.start();
}

void CameraApp::run() {

  JsonDocument doc;
  read_sensors(doc);
  send_json(doc, "health");
  ESP_LOGI(TAG, "Health report published!");

  // config-ok check, or new config loading

  _cam.take_image();
  char timestamp[TIMESTAMP_SIZE] = {0};
  Time::get_date(timestamp, sizeof(timestamp));
  ESP_LOGI(TAG, "Timestamp: %s", timestamp);
  doc.clear();
  std::string header;
  doc["timestamp"] = timestamp;
  doc["size"] = _cam.get_image_size();
  doc["mode"] = "gray";
  serializeJson(doc, header);
  _mqtt.publish("image", header.c_str(), header.size());
  if (_mqtt.wait_for_header_ack(timestamp)) {
    // Timestamp matches, proceed with sending image
    if (_mqtt.publish("image", _cam.get_image_data(), _cam.get_image_size()) !=
        ESP_OK) {
      ESP_LOGE(TAG, "Failed to publish image!");
    } else {
      ESP_LOGI(TAG, "Image published!");
    }
  } else {
    ESP_LOGE(TAG, "No matching timestamp received, skipping image publish!");
  }
  _cam.return_fb();

  // deinit
  deinit_components();
  // sleep
  TimingConfig config = Config::get_active_config();
  mysleep(config.period);
}

esp_err_t CameraApp::read_sensors(JsonDocument &doc) {
  doc["cpuTemp"] = 25.0;
  doc["batteryCharge"] = 100;
  doc["batteryTemp"] = 25.0;
  doc["lightLevel"] = 1500;
  return ESP_OK;
}

esp_err_t CameraApp::send_json(JsonDocument &doc, const char *topic) {
  std::string json;
  serializeJson(doc, json);
  return _mqtt.publish(topic, json.c_str(), json.size());
}