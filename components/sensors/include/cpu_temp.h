#pragma once

#include "driver/temperature_sensor.h"
#include "isensor.h"

/**
 * @brief
 * CPU temperature sensor class
 *
 */
class CpuTemp : public ISensor {
public:
  CpuTemp() = default;
  ~CpuTemp() {
    if (_temp_handle != nullptr) {
      temperature_sensor_uninstall(_temp_handle);
    }
  };

  /**
   * @brief
   * Initializes the CPU temperature sensor.
   *
   * @return
   * ESP_OK if the sensor was initialized successfully, ESP_FAIL otherwise.
   *
   */
  esp_err_t init() override;
  /**
   * @brief
   * Reads the CPU temperature.
   *
   * @return
   * ESP_OK if the temperature was read successfully, ESP_FAIL otherwise.
   *
   */
  esp_err_t read() override;
  /**
   * @brief
   * Returns the CPU temperature.
   *
   * @return
   * The CPU temperature in degrees Celsius.
   *
   */
  float get_value() const override;

private:
  float _cpu_temp = 0.0f;
  temperature_sensor_handle_t _temp_handle = nullptr;
};