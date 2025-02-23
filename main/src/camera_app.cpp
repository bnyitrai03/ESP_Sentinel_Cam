#include "camera_app.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mytime.h"
#include "deep_sleep.h"
#include "esp_timer.h"
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
  // Disable lib logging else remote logging dies
  esp_log_level_set("mqtt5_client", ESP_LOG_NONE);
  esp_log_level_set("mqtt_client", ESP_LOG_NONE);
  _mqtt.start();
}

void CameraApp::run() {

  JsonDocument doc;
  read_sensors(doc);
  std::string health_report;
  serializeJson(doc, health_report);
  _mqtt.publish(_mqtt.get_health_report_topic(), health_report.c_str(), health_report.size());
  ESP_LOGI(TAG, "Health report published!");

  // config-ok check, or new config loading

  _cam.take_image();
  char timestamp[TIMESTAMP_SIZE] = {0};
  Time::get_date(timestamp, sizeof(timestamp));
  ESP_LOGI(TAG, "Timestamp: %s", timestamp);
  JsonDocument doc;
  std::string header;
  doc["timestamp"] = timestamp;
  doc["size"] = _cam.get_image_size();
  doc["mode"] = "gray";
  serializeJson(doc, header);
  _mqtt.publish("image", header.c_str(), header.size());
  if (_mqtt.wait_for_header_ack(timestamp)) {
    // Timestamp matches, proceed with sending image
    _mqtt.publish("image", _cam.get_image_data(), _cam.get_image_size());
    ESP_LOGI(TAG, "Image published!");
  } else {
    ESP_LOGE(TAG, "No matching timestamp received, skipping image publish!");
  }
  _cam.return_fb();

  // deinit, deepsleep
  uint64_t elapsed_time = esp_timer_get_time(); // try measuring this
  TimingConfig _config = Config::get_active_config();
  uint64_t sleep_time = _config.period - elapsed_time/1000000; // maybe need some extra value: flash shutdown/startup
  if(sleep_time > DEEP_SLEEP_THRESHOLD){
    ESP_LOGI(TAG, "Deep sleep for %llu seconds", sleep_time);
  }else if (sleep_time > 0){
    ESP_LOGI(TAG, "Light sleep for %llu seconds", sleep_time);
  }
  else{
    ESP_LOGI(TAG, "No sleep, continuing");
  }
}

esp_err_t CameraApp::read_sensors(JsonDocument &doc) {
  doc["cpuTemp"] = 25.0;
  doc["batteryCharge"] = 100;
  doc["batteryTemp"] = 25.0;
  doc["lightLevel"] = 1500;
  return ESP_OK;
}