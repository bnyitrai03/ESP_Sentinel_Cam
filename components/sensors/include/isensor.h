#pragma once
#include <esp_err.h>

class ISensor {
public:
  virtual ~ISensor() = default;
  /**
   * @brief
   * Initializes the sensor.
   *
   * @return
   * ESP_OK if initialization was successful, ESP_FAIL otherwise.
   *
   */
  virtual esp_err_t init() = 0;
  /**
   * @brief
   * Reads the sensor value.
   *
   * @return
   * ESP_OK if reading was successful, ESP_FAIL otherwise.
   *
   */
  virtual esp_err_t read() = 0;
  /**
   * @brief
   * Returns the sensor value.
   *
   * @return
   * The sensor value.
   *
   */
  virtual float get_value() const = 0;
};