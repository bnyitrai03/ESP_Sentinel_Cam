#pragma once

#include "esp_err.h"
#include <string>

/**
 * @brief Manages configuration settings.
 */
class Storage {
public:
  /**
   * @brief Constructor for Storage class
   *
   * Initializes the NVS flash storage.
   */
  Storage();

  /**
   * @brief Writes a key-value pair to the NVS storage
   *
   * @param key The key to write
   * @param value The value to write
   * @return
   *     - ESP_OK on success
   *     - ESP_FAIL on error
   */
  static esp_err_t write(const std::string &key, const std::string &value);

  /**
   * @brief Writes a key-value pair to the NVS storage
   *
   * @param key The key to write
   * @param value The value to write
   * @return
   *     - ESP_OK on success
   *
   *     - ESP_FAIL on error
   */
  static esp_err_t write(const std::string &key, uint32_t value);

  /**
   * @brief Reads a value from the NVS storage
   *
   * @param key The key to read
   * @param value The buffer to store the read value
   * @param len The length of the buffer
   * @return
   *     - ESP_OK on success
   *     - ESP_FAIL on error
   */
  static esp_err_t read(const std::string &key, char *value, uint32_t len);

  /**
   * @brief Reads the error_count value from the NVS storage
   *
   * @note If the error_count value does not exist, it will be created with a
   * value of 1.
   *
   * @param value The buffer to store the read value
   * @return
   *     - ESP_OK on success
   *
   *     - ESP_FAIL on error
   */
  static esp_err_t read_error_count(uint32_t *value);

  /**
   * @brief Erases the NVS storage
   */
  static void erase_nvs();
};
