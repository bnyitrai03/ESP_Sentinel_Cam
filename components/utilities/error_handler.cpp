#include "error_handler.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "led.h"
#include "mysleep.h"
#include "storage.h"

constexpr auto TAG = "error_handler";

constexpr uint64_t ERROR_SLEEP_TIME = 600; // 10 minutes

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
  ESP_LOGE(TAG, "There was an error, restarting the device...");
  Led::set_pattern(Led::Pattern::ERROR_BLINK);
  uint32_t error_count = get_error_count();
  vTaskDelay(10000 / portTICK_PERIOD_MS);

  deinit_components();

  if (error_count == 15) {
    mysleep(ERROR_SLEEP_TIME);
  } else {
    esp_restart();
  }
}

void reset_device() {
  Storage::erase_nvs();
  ESP_LOGW(TAG, "Device reset complete, restarting...");
  esp_restart();
}

void deinit_components() {
  vTaskDelay(300 / portTICK_PERIOD_MS);
  if (mqtt_deinit_callback != nullptr) {
    mqtt_deinit_callback();
  }
  if (camera_deinit_callback != nullptr) {
    camera_deinit_callback();
  }
  if (wifi_deinit_callback != nullptr) {
    wifi_deinit_callback();
  }
}

uint32_t get_error_count() {
  uint32_t error_count = 0;
  Storage::read_error_count(&error_count);
  error_count++;
  if (error_count >= 16) {
    error_count = 0;
  }
  if (error_count == 15) {
    ESP_LOGE(TAG, "Error count has reached max limit, device going to sleep "
                  "for 10 minutes");
  }
  Storage::write("error_count", error_count);

  return error_count;
}