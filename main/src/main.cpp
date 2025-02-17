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
#include "config.h"
#include "error_handler.h"
#include "mqtt.h"
#include "mytime.h"
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

  // ------ Dynamic config values ------------------
  {
    JsonDocument doc;
    char config[512];
    doc["uuid"] = "8D8AC610-566D-4EF0-9C22-186B2A5ED793";
    JsonArray timing = doc["timing"].to<JsonArray>();
    JsonObject timing_0 = timing.add<JsonObject>();
    timing_0["period"] = 40;
    timing_0["start"] = "00:00:00";
    timing_0["end"] = "23:59:59";
    serializeJson(doc, config);
    Storage::write("static_config", config);
  }
  // -----------------------------------------------

  Config config;

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

    char timestamp[TIMESTAMP_SIZE] = {0};
    Time::get_utc_timestamp(timestamp, sizeof(timestamp));

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