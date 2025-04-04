#pragma once

#include "driver/rtc_io.h"
#include "i2c_manager.h"
#include "isensor.h"

/**
 * @brief Light sensor class implementing the ISensor interface for the OPT3005
 * ambient light sensor.
 */
class LightSensor : public ISensor {
public:
  /**
   * @brief Constructor for LightSensor.
   * @param _i2c Reference to an I2CManager instance for I2C communication.
   */
  LightSensor(I2CManager &_i2c);

  /**
   * @brief Destructor for LightSensor.
   * Cleans up I2C device resources.
   */
  ~LightSensor();

  /**
   * @brief Initializes the light sensor.
   * @return esp_err_t ESP_OK on success, error code otherwise.
   */
  esp_err_t init() override;

  /**
   * @brief Reads the current light level from the sensor.
   * @return esp_err_t ESP_OK on success, error code otherwise.
   */
  esp_err_t read() override;

  /**
   * @brief Gets the last read light value.
   * @return float The light value in lux, rounded to nearest integer.
   */
  float get_value() const override;

private:
  /**
   * @brief Reads a 16-bit register from the sensor.
   * @param reg The register address to read from.
   * @param value Pointer to store the read value.
   * @return esp_err_t ESP_OK on success, error code otherwise.
   */
  esp_err_t read_register(uint8_t reg, uint16_t *value);

  /**
   * @brief Writes a 16-bit value to a sensor register.
   * @param reg The register address to write to.
   * @param value The value to write.
   * @return esp_err_t ESP_OK on success, error code otherwise.
   */
  esp_err_t write_register(uint8_t reg, uint16_t value);

  I2CManager &_i2c;
  i2c_master_dev_handle_t _device_handle;
  float _light_value = -1.0f;

  // OPT3005 I2C address (0x45)
  const uint16_t OPT3005_ADDRESS = 0x45;

  // OPT3005 register addresses
  const uint16_t OPT3005_RESULT_REG = 0x00;
  const uint16_t OPT3005_CONFIG_REG = 0x01;

  // OPT3005 configuration bits
  const uint16_t OPT3005_CONFIG_RANGE_AUTO = 0xC000;
  const uint16_t OPT3005_CONFIG_CONV_TIME_800MS = 0x0800;
  const uint16_t OPT3005_CONFIG_SINGLE_SHOT = 0x0100;
  const uint16_t OPT3005_CONFIG_OVF = 0x0100;
  const uint16_t OPT3005_CONFIG_CONV_READY = 0x0080;
};