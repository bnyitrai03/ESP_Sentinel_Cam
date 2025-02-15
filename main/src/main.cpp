#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <ArduinoJson.h>
#include <esp_log.h>
#include <esp_system.h>
#include <sys/param.h>

#ifndef portTICK_RATE_MS
#define portTICK_RATE_MS portTICK_PERIOD_MS
#endif

#include "camera.h"
#include "error_handler.h"
#include "mqtt.h"
#include "secret.h"
#include "storage.h"
#include "wifi.h"

constexpr auto *TAG = "main_app";

extern "C" void app_main(void) {
  ESP_LOGI(TAG, "Free PSRAM before init: %d",
           heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
  ESP_LOGI(TAG, "Largest free block in PSRAM: %d",
           heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));

  Storage storage;
  // ------ Static config values ------------------
  Storage::write("ssid", WIFI_SSID);
  Storage::write("password", WIFI_PASS);

  Storage::write("mqttAddress", MQTT_BROKER_IP);
  Storage::write("mqttUser", MQTT_USERNAME);
  Storage::write("mqttPassword", MQTT_PASSWORD);
  Storage::write("imageTopic", IMAGE_TOPIC);
  Storage::write("imageAckTopic", IMAGE_ACK_TOPIC);
  Storage::write("healthRepTopic", HEALTH_REPORT_TOPIC);
  Storage::write("configAckTopic", HEALTH_REPORT_RESP_TOPIC);
  Storage::write("logTopic", LOG_TOPIC);
  // -----------------------------------------------

  Wifi wifi;
  wifi.init();
  wifi.connect();

  // Disable lib logging else remote logging dies
  esp_log_level_set("mqtt5_client", ESP_LOG_NONE);
  esp_log_level_set("mqtt_client", ESP_LOG_NONE);
  MQTT mqtt;
  mqtt.start();

  Camera cam;
  cam.start();

  ESP_LOGI(TAG, "Free PSRAM after init: %d",
           heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
  ESP_LOGI(TAG, "Largest free block in PSRAM: %d",
           heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));

  while (1) {
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    cam.take_image();

    time_t now;
    time(&now);
    struct tm timeinfo;
    gmtime_r(&now, &timeinfo); // Convert to UTC time
    char timestamp[TIMESTAMP_SIZE] = {0};
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%SZ", &timeinfo);

    JsonDocument doc;
    std::string output;
    doc["timestamp"] = timestamp;
    doc["size"] = cam.get_image_size();
    serializeJson(doc, output);
    mqtt.publish("image", output.c_str(), output.size());
    if (mqtt.wait_for_sendack(timestamp)) {
      // Timestamp matches, proceed with sending image
      mqtt.publish("image", cam.get_image_data(), cam.get_image_size());
      ESP_LOGI(TAG, "Image published!");
    } else {
      ESP_LOGE(TAG, "No matching timestamp received, skipping image publish!");
    }
    cam.return_fb();
  }
}