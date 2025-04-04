#pragma once

#include "battery_manager.h"
#include "isensor.h"
#include <esp_err.h>

/**
 * @brief
 * BatteryCharge class for calculating battery charge percentage
 */
class BatteryCharge : public ISensor {
public:
  /**
   * @brief Construct a new BatteryCharge object
   */
  BatteryCharge(BatteryManager &battery_manager);

  /**
   * @brief Initialize the sensor
   *
   * @return ESP_OK if successful, otherwise an error code
   */
  esp_err_t init() override;

  /**
   * @brief Read the battery charge percentage
   *
   * @return ESP_OK if successful, otherwise an error code
   */
  esp_err_t read() override;

  /**
   * @brief Get the battery charge percentage value
   *
   * @return float Battery charge percentage (0-100)
   */
  float get_value() const override { return _charge_percentage; }

private:
  /**
   * @brief Calculate charge percentage from voltage
   *
   * @param voltage Battery voltage in Volts
   * @return float Charge percentage (0-100)
   */
  float calculate_percentage_from_voltage(float voltage);

  BatteryManager &_battery_manager;
  float _charge_percentage;

  // Lookup tables for voltage to percentage conversion
  const float _voltages[14] = {4.2f, 4.1f,  4.0f,  3.9f,  3.82f, 3.75f, 3.72f,
                               3.7f, 3.68f, 3.65f, 3.62f, 3.55f, 3.4f,  3.0f};
  const float _percentages[14] = {100.0f, 95.0f, 90.0f, 85.0f, 80.0f,
                                  70.0f,  60.0f, 50.0f, 40.0f, 30.0f,
                                  20.0f,  15.0f, 10.0f, 0.0f};
  const uint16_t _table_size = 14;
};