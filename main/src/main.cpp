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
  Storage::write("configTopic", HEALTH_REPORT_RESP_TOPIC);
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
    timing_0["end"] = "10:59:59";
    JsonObject timing_1 = timing.add<JsonObject>();
    timing_1["period"] = -1;
    timing_1["start"] = "11:00:00";
    timing_1["end"] = "15:59:59";
    JsonObject timing_2 = timing.add<JsonObject>();
    timing_2["period"] = 30;
    timing_2["start"] = "16:00:00";
    timing_2["end"] = "23:59:59";

    serializeJson(doc, config);
    Storage::write("static_config", config);
  }
  // -----------------------------------------------

  Wifi wifi;
  wifi.init();
  wifi.connect();

  Config config;

  // Disable lib logging else remote logging dies
  esp_log_level_set("mqtt5_client", ESP_LOG_NONE);
  esp_log_level_set("mqtt_client", ESP_LOG_NONE);
  MQTT mqtt;
  mqtt.start();

  Camera cam;
  cam.start();

  while (1) {
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    ESP_LOGI(TAG, "Hello World!");
    /* cam.take_image();

    char timestamp[TIMESTAMP_SIZE] = {0};
    Time::get_date(timestamp, sizeof(timestamp));

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
    cam.return_fb(); */
  }
}