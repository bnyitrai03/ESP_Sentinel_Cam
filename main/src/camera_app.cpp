#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <ArduinoJson.h>
#include <esp_log.h>
#include <esp_system.h>
#include <sys/param.h>

#ifndef portTICK_RATE_MS
#define portTICK_RATE_MS portTICK_PERIOD_MS
#endif

constexpr auto *TAG = "camera_app";

void camera_app() {

  camera_app_init();

  cam.take_image();
  char timestamp[TIMESTAMP_SIZE] = {0};
  Time::get_date(timestamp, sizeof(timestamp));
  ESP_LOGI(TAG, "Timestamp: %s", timestamp);
  JsonDocument doc;
  std::string header;
  doc["timestamp"] = timestamp;
  doc["size"] = cam.get_image_size();
  doc["mode"] = "gray";
  serializeJson(doc, header);
  mqtt.publish("image", header.c_str(), header.size());
  if (mqtt.wait_for_header_ack(timestamp)) {
    // Timestamp matches, proceed with sending image
    mqtt.publish("image", cam.get_image_data(), cam.get_image_size());
    ESP_LOGI(TAG, "Image published!");
  } else {
    ESP_LOGE(TAG, "No matching timestamp received, skipping image publish!");
  }
  cam.return_fb();
}

void camera_app_init() {
  Camera cam;
  cam.start();

  Wifi wifi;
  wifi.init();
  wifi.connect();

  Config config;

  // Disable lib logging else remote logging dies
  esp_log_level_set("mqtt5_client", ESP_LOG_NONE);
  esp_log_level_set("mqtt_client", ESP_LOG_NONE);
  MQTT mqtt;
  mqtt.start();
}