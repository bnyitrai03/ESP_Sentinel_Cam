#pragma once

#include <driver/i2c_master.h>

/**
 * @brief
 * I2C Manager class for handling I2C communication as a singleton.
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
   * Sets up the I2C bus with default configuration (I2C_NUM_0, GPIO 4/5).
   *
   * @return ESP_OK if successful, otherwise an error code from the ESP-IDF
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
   * @brief Write data to a slave device.
   *
   * @param device_handle Handle to the target I2C device.
   * @param data Pointer to the data buffer to write.
   * @param length Number of bytes to write.
   * @param timeout Operation timeout in milliseconds (default: 5000ms).
   * @return esp_err_t ESP_OK on success, error code on failure.
   */
  esp_err_t write(i2c_master_dev_handle_t device_handle, const uint8_t *data,
                  size_t length, int timeout = 5000);

  /**
   * @brief Read data from a slave device.
   *
   * @param device_handle Handle to the target I2C device.
   * @param data Pointer to the buffer where read data will be stored.
   * @param length Number of bytes to read.
   * @param timeout Operation timeout in milliseconds (default: 5000ms).
   * @return esp_err_t ESP_OK on success, error code on failure.
   */
  esp_err_t read(i2c_master_dev_handle_t device_handle, uint8_t *data,
                 size_t length, int timeout = 5000);

  /**
   * @brief Write data to and then read data from a slave device in one
   * operation.
   *
   * @param device_handle Handle to the target I2C device.
   * @param write_data Pointer to the data buffer to write.
   * @param write_length Number of bytes to write.
   * @param read_data Pointer to the buffer where read data will be stored.
   * @param read_length Number of bytes to read.
   * @param timeout Operation timeout in milliseconds (default: 5000ms).
   * @return esp_err_t ESP_OK on success, error code on failure.
   */
  esp_err_t write_and_read(i2c_master_dev_handle_t device_handle,
                           const uint8_t *write_data, size_t write_length,
                           uint8_t *read_data, size_t read_length,
                           int timeout = 5000);

  /**
   * @brief Probe for a device on the I2C bus.
   * Checks if a device with the specified address is connected to the bus.
   *
   * @param address The 7-bit I2C address of the device to probe.
   * @param timeout Operation timeout in milliseconds (default: 5000ms).
   */
  void probe(uint16_t address, int timeout = 5000);

private:
  i2c_master_bus_handle_t _bus_handle;
  bool _initialized;
};