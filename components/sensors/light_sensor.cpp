#include "light_sensor.h"
#include "esp_log.h"
#include <algorithm>
#include <cmath>

auto constexpr TAG = "LightSensor";

float LightSensor::convert_adc_reading(int raw_value) {
  // Convert ADC reading to voltage
  float voltage = (raw_value / ADC_MAX) * V_MAX + _offset;
  // Convert voltage to resistance
  // Voltage divider formula: R_LDR = R1 * (V_MAX/V_OUT - 1)
  const float R1 = 10000.0f; // 10k resistor
  float resistance = R1 * (V_MAX / voltage - 1);

  // Convert resistance to lux using the formula: Lux = 10 * (R10 / R)^(1/γ)
  const float R10 = 14000.0f; // Average resistance at 10 lux
  const float gamma = 0.7f;   // Gamma value for the LDR
  float lux = 10 * pow(R10 / resistance, 1.0f / gamma);

  // Clamp lux value between 0 and 100000
  lux = std::max(0.0f, std::min(lux, 100000.0f));

  ESP_LOGI(TAG,
           "ADC: %d, Voltage: %.2fV, Resistance: %.0f ohm, Light: %.1f lux",
           raw_value, voltage, resistance, lux);
  return static_cast<int>(lux);
}