#pragma once

#include "battery_manager.h"
#include "i2c_manager.h"
#include "isensor.h"
#include <ArduinoJson.h>
#include <esp_err.h>
#include <map>
#include <memory>

/**
 * @brief
 * Sensors class
 *
 */
class Sensors {
public:
  /**
   * @brief
   * Instantiate the Sensors
   *
   */
  Sensors();

  /**
   * @brief
   * Initialize the sensors
   *
   * @return
   * ESP_OK if successful, otherwise an error code
   *
   */
  esp_err_t init();
  /**
   * @brief
   * Read the sensor values
   *
   * @param doc
   * The JSON document to store the sensor values
   *
   * @return
   * ESP_OK if successful, otherwise an error code
   *
   */
  esp_err_t read_sensors(JsonDocument &doc);

  esp_err_t read_battery_after_cam_start(int16_t *current);

  void reset_i2c_and_bq();

  void enable_ADC() { _battery_manager.enable_ADC(); }

private:
  std::map<std::string, std::unique_ptr<ISensor>> _sensors;
  I2CManager _i2c_manager;
  BatteryManager _battery_manager;
};