#pragma once

#include "i2c_manager.h"
#include <cstdint>
#include <esp_err.h>
#include <memory>

/**
 * @brief
 * BatteryManager class for managing the bq25622 battery charging IC
 */
class BatteryManager {
public:
  /**
   * @brief Constructor that takes an I2CManager reference for dependency
   * injection
   *
   * @param i2c Reference to an I2CManager instance
   */
  BatteryManager(I2CManager &i2c);

  /**
   * @brief Destructor
   */
  ~BatteryManager();

  /**
   * @brief Initialize the BatteryManager and setup I2C communication
   *
   * @return ESP_OK if successful, otherwise an error code
   */
  esp_err_t init();

  /**
   * @brief Get the battery voltage in Volts
   *
   * @param voltage Pointer to store the battery voltage
   * @return ESP_OK if successful, otherwise an error code
   */
  esp_err_t get_battery_voltage(float *voltage);

  /**
   * @brief Get the battery temperature in  percentage of bias reference
   *
   * @param temperature Pointer to store the battery temperature
   * @return ESP_OK if successful, otherwise an error code
   */
  esp_err_t get_battery_temperature(float *temperature);

  /**
   * @brief Get the charge current in milliamps
   *
   * @param current Pointer to store the charge current
   * @return ESP_OK if successful, otherwise an error code
   */
  esp_err_t get_charge_current(int16_t *current);

  /**
   * @brief Disable the ADC in the BQ25622
   */
  void disable_ADC();

  /**
   * @brief Enable the ADC in the BQ25622
   */
  void enable_ADC();

  // TODO: remove this
  bool _initialized;
  bool measure_adc_enabled = true;

private:
  /**
   * @brief Read a register from the BQ25622
   *
   * @param reg Register address
   * @param data Pointer to store the read data
   * @return ESP_OK if successful, otherwise an error code
   */
  esp_err_t read_register(uint8_t reg, uint16_t *data);

  /**
   * @brief Write a register to the BQ25622
   *
   * @param reg Register address
   * @param data Data to write
   * @return ESP_OK if successful, otherwise an error code
   */
  esp_err_t write_register(uint8_t reg, uint8_t data);

  I2CManager &_i2c;
  i2c_master_dev_handle_t _device_handle;

  // BQ25622 I2C Address
  const uint8_t BQ25622_ADDR = 0x6B;

  // BQ25622 Register Map
  const uint8_t REG_ADC_CONTROL = 0x26;
  const uint8_t REG_VBAT_READ = 0x30;
  const uint8_t REG_TEMP_READ = 0x34;
  const uint8_t REG_CHARGE_CURRENT = 0x2A;

  // ADC config bits
  const uint8_t ADC_ENABLE = 0x80;
  const uint8_t ADC_AVG = 0x08;
  const uint8_t ADC_AVG_INIT = 0x04;
  const uint8_t ADC_SAMPLE = 0xDF; // 11 bit effective resolution
  const uint8_t ADC_RATE = 0x40;   // one shot mode
};