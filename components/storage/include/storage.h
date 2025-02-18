#include <nvs_flash.h>
#include <string>

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
};
