#pragma once

#include <driver/i2c_master.h>

/**
 * @brief
 * I2C Manager class for handling I2C communication as a singleton
 */
class I2CManager {
public:
  /**
   * @brief
   */
  I2CManager();

  /**
   * @brief
   * Destructor - cleans up I2C driver if initialized
   */
  ~I2CManager();

  /**
   * @brief
   * Initialize the I2C driver
   *
   * @return ESP_OK if successful, otherwise an error code
   */
  esp_err_t init();

  /**
   * @return I2C bus handle
   *
   */
  i2c_master_bus_handle_t get_bus_handle() { return _bus_handle; }

  /**
   * @brief Write data to the slave device
   *
   * @param device_handle
   * @param data
   * @param length
   * @param timeout
   *
   * @return esp_err_t
   */
  esp_err_t write(i2c_master_dev_handle_t device_handle, const uint8_t *data,
                  size_t length, int timeout = 5000);

  /**
   * @brief Read data from the slave device
   *
   * @param device_handle
   * @param data
   * @param length
   * @param timeout
   * @return esp_err_t
   */
  esp_err_t read(i2c_master_dev_handle_t device_handle, uint8_t *data,
                 size_t length, int timeout = 5000);

  /**
   * @brief
   *
   * @param device_handle
   * @param write_data
   * @param write_length
   * @param read_data
   * @param read_length
   * @param timeout
   * @return esp_err_t
   */
  esp_err_t write_and_read(i2c_master_dev_handle_t device_handle,
                           const uint8_t *write_data, size_t write_length,
                           uint8_t *read_data, size_t read_length,
                           int timeout = 5000);

  /**
   * @brief Detect whether the specific device has been connected to the I2C bus
   *
   * @param address
   * @param timeout
   */
  void probe(uint16_t address, int timeout = 5000);

private:
  i2c_master_bus_handle_t _bus_handle;
  bool _initialized;
};