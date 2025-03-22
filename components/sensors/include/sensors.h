#pragma once

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

private:
  std::map<std::string, std::unique_ptr<ISensor>> _sensors;
};