#include "error_handler.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"

static DeinitCallback wifi_deinit_callback = nullptr;
static DeinitCallback mqtt_deinit_callback = nullptr;
static DeinitCallback camera_deinit_callback = nullptr;

void set_wifi_deinit_callback(DeinitCallback callback) {
  wifi_deinit_callback = callback;
}

void set_mqtt_deinit_callback(DeinitCallback callback) {
  mqtt_deinit_callback = callback;
}

void set_camera_deinit_callback(DeinitCallback callback) {
  camera_deinit_callback = callback;
}

void restart() {
  ESP_LOGW("error_handler", "Restarting the device...");
  vTaskDelay(2000 / portTICK_PERIOD_MS); // To send the final log
  deinit_components();
  esp_restart();
}

void deinit_components() {
  vTaskDelay(100 / portTICK_PERIOD_MS);
  if (mqtt_deinit_callback) {
    mqtt_deinit_callback();
  }
  if (camera_deinit_callback) {
    camera_deinit_callback();
  }
  if (wifi_deinit_callback) {
    wifi_deinit_callback();
  }
}
