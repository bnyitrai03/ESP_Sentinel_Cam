#pragma once

#include "battery_manager.h"
#include "isensor.h"
#include <esp_err.h>

/**
 * @brief
 * ChargeCurrent class for reading battery charge current
 */
class ChargeCurrent : public ISensor {
public:
  /**
   * @brief Construct a new ChargeCurrent object
   *
   * @param battery_manager Reference to the BatteryManager instance
   */
  ChargeCurrent(BatteryManager &battery_manager);

  /**
   * @brief Initialize the sensor
   *
   * @return ESP_OK if successful, otherwise an error code
   */
  esp_err_t init() override;

  /**
   * @brief Read the charge current
   *
   * @return ESP_OK if successful, otherwise an error code
   */
  esp_err_t read() override;

  /**
   * @brief Get the charge current value in milliamps
   *
   * @return float Charge current in milliamps
   */
  float get_value() const override { return _charge_current; }

private:
  BatteryManager &_battery_manager;
  int16_t _charge_current;
};