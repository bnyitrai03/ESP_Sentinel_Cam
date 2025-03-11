#include "battery_charge.h"
#include "esp_log.h"
#include <cmath>

auto constexpr TAG = "BatteryCharge";

float BatteryCharge::convert_adc_reading(int raw_value) {
  // divider with 10k:10k ratio doubles the measurable range
  const float VOLTAGE_DIVIDER_RATIO = 2.0f;
  // Convert ADC reading to voltage and account for voltage divider
  float measured_voltage =
      (raw_value / ADC_MAX) * V_MAX * VOLTAGE_DIVIDER_RATIO + _offset;

  // Lookup tables for voltage to percentage conversion
  const float voltages[] = {4.2f, 4.1f,  4.0f,  3.9f,  3.82f, 3.75f, 3.72f,
                            3.7f, 3.68f, 3.65f, 3.62f, 3.55f, 3.4f,  3.0f};
  const float percentages[] = {100.0f, 95.0f, 90.0f, 85.0f, 80.0f, 70.0f, 60.0f,
                               50.0f,  40.0f, 30.0f, 20.0f, 15.0f, 10.0f, 0.0f};
  const int table_size = sizeof(voltages) / sizeof(voltages[0]);

  // Find the closest voltage in the lookup table
  float battery_percentage = 0.0f;

  if (measured_voltage >=
      voltages[0]) { // If voltage is higher than the highest
    battery_percentage = percentages[0];
  } else if (measured_voltage <=
             voltages[table_size - 1]) { // If voltage is lower than the lowest
    battery_percentage = percentages[table_size - 1];
  } else {
    // Linear interpolation between voltage points
    for (int i = 0; i < table_size - 1; i++) {
      if (measured_voltage <= voltages[i] &&
          measured_voltage > voltages[i + 1]) {
        float voltage_diff = voltages[i] - voltages[i + 1];
        float percentage_diff = percentages[i] - percentages[i + 1];
        float ratio = (measured_voltage - voltages[i + 1]) / voltage_diff;
        battery_percentage = percentages[i + 1] + (ratio * percentage_diff);
        break;
      }
    }
  }

  ESP_LOGI(TAG, "ADC: %d, Voltage: %.2fV, Battery: %.1f%%", raw_value,
           measured_voltage, battery_percentage);
  return std::round(battery_percentage * 100) / 100;
}