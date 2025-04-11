#pragma once

#include "battery_manager.h"
#include "isensor.h"
#include <esp_err.h>

/**
 * @brief
 * BatteryTemp class for reading battery temperature
 */
class BatteryTemp : public ISensor {
public:
  /**
   * @brief Construct a new BatteryTemp object
   *
   * @param battery_manager Reference to the BatteryManager instance
   */
  BatteryTemp(BatteryManager &battery_manager);

  /**
   * @brief Initialize the sensor
   *
   * @return ESP_OK if successful, otherwise an error code
   */
  esp_err_t init() override;

  /**
   * @brief Read the battery temperature
   *
   * @return ESP_OK if successful, otherwise an error code
   */
  esp_err_t read() override;

  /**
   * @brief Get the battery temperature value in degrees Celsius
   *
   * @return float Battery temperature in degrees Celsius
   */
  float get_value() const override { return _temperature; }

private:
  /**
   * @brief Convert the temperature to degrees Celsius
   *
   * @param percentage The percentage of bias reference value
   * @return float Temperature in degrees Celsius
   */
  float convert_to_degrees(float percentage);

  BatteryManager &_battery_manager;
  float _temperature;

  const float min_temp = -40.0f;
  const float max_temp = 85.0f;
  const float min_percent = 20.9f;
  const float max_percent = 83.2f;
};