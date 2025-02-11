#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
/* #include "driver/i2c_master.h"
#include "driver/temperature_sensor.h" */

#include <ArduinoJson.h>

#ifndef portTICK_RATE_MS
#define portTICK_RATE_MS portTICK_PERIOD_MS
#endif

#include "camera.h"
#include "mqtt.h"
#include "secret.h"
#include "wifi.h"

constexpr auto *TAG = "main_app";

extern "C" void app_main(void) {
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  ESP_LOGI(TAG, "Free PSRAM before init: %d",
           heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
  ESP_LOGI(TAG, "Largest free block in PSRAM: %d",
           heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));

  Wifi wifi;
  wifi.init();
  wifi.connect(WIFI_SSID, WIFI_PASS);

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

  time_t now;
  time(&now);
  ESP_LOGI(TAG, "Unix Epoch Time: %lld", (unsigned long long)now);

  while (1) {
    vTaskDelay(5000 / portTICK_PERIOD_MS);

    cam.take_image();
    mqtt.publish("mqtt/rpi/image", cam.get_image_data(), cam.get_image_size());
    ESP_LOGI(TAG, "Message published!");
    cam.return_fb();
  }
}