#include "cpu_temp.h"
#include "esp_log.h"
#include <cmath>

auto constexpr *TAG = "CpuTemp";

esp_err_t CpuTemp::init() {
  temperature_sensor_config_t temp_sensor_config =
      TEMPERATURE_SENSOR_CONFIG_DEFAULT(-10, 80);
  return temperature_sensor_install(&temp_sensor_config, &_temp_handle);
}

esp_err_t CpuTemp::read() {
  esp_err_t err = temperature_sensor_enable(_temp_handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to enable temperature sensor!");
    return err;
  }

  err = temperature_sensor_get_celsius(_temp_handle, &_cpu_temp);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to read temperature sensor!");
    return err;
  }
  ESP_LOGI(TAG, "CPU temperature: %.2f°C", _cpu_temp);

  err = temperature_sensor_disable(_temp_handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to disable temperature sensor!");
    return err;
  }
  return ESP_OK;
}

float CpuTemp::get_value() const { return std::round(_cpu_temp * 100) / 100; }