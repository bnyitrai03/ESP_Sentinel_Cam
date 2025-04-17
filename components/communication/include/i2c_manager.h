#pragma once

#include <driver/i2c_master.h>

/**
 * @brief
 * I2C Manager class for handling I2C communication.
 * Provides a high-level interface for I2C operations including initialization,
 * read/write operations, and device probing.
 */
class I2CManager {
public:
  /**
   * @brief Default constructor.
   * Initializes the I2C manager in a non-initialized state.
   */
  I2CManager();

  /**
   * @brief Destructor.
   * Cleans up I2C driver resources if initialized.
   */
  ~I2CManager();

  /**
   * @brief Initialize the I2C driver.
   * Sets up the I2C bus with default configuration (I2C_NUM_0, GPIO 4 and 5).
   *
   * @return ESP_OK if successful, otherwise an error code
   * framework.
   */
  esp_err_t init();

  /**
   * @brief Get the I2C bus handle.
   *
   * @return i2c_master_bus_handle_t The handle to the I2C bus.
   */
  i2c_master_bus_handle_t get_bus_handle() { return _bus_handle; }

  /**
   * @brief Probe for a device on the I2C bus.
   * Checks if a device with the specified address is connected to the bus.
   *
   * @param address The 7-bit I2C address of the device to probe.
   * @param timeout Operation timeout in milliseconds (default: 5000ms).
   *
   * @return ESP_OK if the device is found, otherwise an error code
   */
  esp_err_t probe(uint16_t address, int timeout = 5000);

  /**
   * @brief Reset the I2C bus.
   * Resets the I2C bus to a known state.
   *
   * @return ESP_OK if successful, otherwise an error code
   */
  esp_err_t reset();

private:
  i2c_master_bus_handle_t _bus_handle;
  bool _initialized;
};