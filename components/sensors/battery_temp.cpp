#include "battery_temp.h"
#include "esp_log.h"

constexpr auto *TAG = "BatteryTemp";

BatteryTemp::BatteryTemp(BatteryManager &battery_manager)
    : _battery_manager(battery_manager), _temperature(0.0f) {
  esp_log_level_set("temperature_sensor", ESP_LOG_WARN);
}

esp_err_t BatteryTemp::init() { return _battery_manager.init(); }

esp_err_t BatteryTemp::read() {
  esp_err_t err = _battery_manager.get_battery_temperature(&_temperature);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to read battery temperature: %d", err);
    return err;
  }

  _temperature = convert_to_degrees(_temperature);
  ESP_LOGI(TAG, "Battery temperature: %.1f °C", _temperature);
  return ESP_OK;
}

float BatteryTemp::convert_to_degrees(float percentage) {
  if (percentage < min_percent) {
    return min_temp;
  }

  if (percentage > max_percent) {
    return max_temp;
  }

  // TODO: Calculate this more accurately
  // Linear interpolation
  float temperature =
      min_temp + ((percentage - min_percent) * (max_temp - min_temp)) /
                     (max_percent - min_percent);
  return temperature;
}