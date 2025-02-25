#include "camera_app.h"
#include "esp_log.h"
#include "secret.h"
#include "storage.h"
#include <ArduinoJson.h>
#include <esp_log.h>

constexpr auto *TAG = "main_app";

extern "C" void app_main(void) {
  Storage storage;

  // -------------- Static config values ------------------
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

  // ------------- Dynamic config values ------------------
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
    timing_1["period"] = 50;
    timing_1["start"] = "11:00:00";
    timing_1["end"] = "14:21:40";
    JsonObject timing_2 = timing.add<JsonObject>();
    timing_2["period"] = 30;
    timing_2["start"] = "14:21:40";
    timing_2["end"] = "23:59:59";

    serializeJson(doc, config);
    Storage::write("dynamic_config", config);
  }

  // Start led state display task
  // Start button handling task

  // ------------------- Application select ---------------------------
  Storage::write("app", "cam"); // this value is empty, when booting in qr mode

  char app_mode[4] = {0};
  Storage::read("app", app_mode, sizeof(app_mode));
  if (strcmp(app_mode, "cam") == 0) {
    ESP_LOGI(TAG, "Starting the starling detection mode");
    CameraApp &app = CameraApp::getInstance();
    app.run();
  } else {
    ESP_LOGI(TAG, "Starting the QR code reader mode");
  }
  // ------------------------------------------------------------------
}