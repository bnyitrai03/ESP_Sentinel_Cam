#include "battery_charge.h"
#include "esp_log.h"
#include <cmath>

constexpr auto *TAG = "BatteryCharge";

BatteryCharge::BatteryCharge(BatteryManager &battery_manager)
    : _battery_manager(battery_manager), _charge_percentage(-1.0f) {}

esp_err_t BatteryCharge::init() { return _battery_manager.init(); }

esp_err_t BatteryCharge::read() {
  float voltage = 0.0f;
  esp_err_t ret = _battery_manager.get_battery_voltage(&voltage);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to read battery voltage: %s", esp_err_to_name(ret));
    return ret;
  }

  _charge_percentage = calculate_percentage_from_voltage(voltage);
  ESP_LOGI(TAG, "Battery voltage: %.2f V, Charge percentage: %.2f%%", voltage,
           _charge_percentage);
  return ESP_OK;
}

float BatteryCharge::calculate_percentage_from_voltage(float voltage) {
  if (voltage >= _voltages[0]) {
    return _percentages[0];
  }

  if (voltage <= _voltages[_table_size - 1]) {
    return _percentages[_table_size - 1];
  }

  for (size_t i = 0; i < _table_size - 1; ++i) {
    if (voltage <= _voltages[i] && voltage > _voltages[i + 1]) {
      // Calculate interpolation ratio
      float voltage_range = _voltages[i] - _voltages[i + 1];
      float percentage_range = _percentages[i] - _percentages[i + 1];
      float position_in_range = (voltage - _voltages[i + 1]) / voltage_range;

      // Interpolate and round to 2 decimal places
      float interpolated_percentage =
          _percentages[i + 1] + (position_in_range * percentage_range);
      float rounded_percentage =
          std::round(interpolated_percentage * 100) / 100;
      return rounded_percentage;
    }
  }

  // This line should theoretically never be reached due to edge case checks
  ESP_LOGE(TAG, "Voltage conversion error!");
  return -1.0f;
}