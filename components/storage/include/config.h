#pragma once

#include "json_converters.h"
#include "mytime.h"
#include <vector>

/**
 * @brief Structure to hold timing configuration.
 */
typedef struct {
  int64_t period; /*!< working period of the device, -1: sleeping */
  Time start;     /*!< HH:MM:SS */
  Time end;       /*!< HH:MM:SS */
} TimingConfig;

/**
 * @brief Manages configuration settings.
 */
class Config {
public:
  /**
   * @brief Loads the configuration from a JSON document and sets the active
   * configuration as well as the UUID
   *
   * @param config The new configuration as a JSON document
   *
   */
  static void load_config(JsonDocument &config);

  /**
   * @brief Loads the configuration from the NVS storage, handles validation
   * too
   *
   * @note This function will restart the device if the configuration is invalid
   *
   */
  static void load_from_storage();

  /**
   * @brief Validates the configuration
   *
   *
   * @param config The new configuration as a string
   *
   * @return
   *     - True if the configuration is valid
   *
   *     - False if the configuration is invalid
   *
   */
  static bool validate(JsonDocument &config);

  /**
   * @brief Gets the active configuration
   *
   * @note If the active config is not found, the default config is returned
   *
   * @return
   *    - The active configuration
   */
  static TimingConfig get_active_config();

  /**
   * @brief Sets the active configuration based on the current time
   *
   * @note If the active config is not found, the default config is set
   *
   * @return -1 if the active config period is -1 (sleeping)
   *
   */
  static int32_t set_active_config();

  /**
   * @brief Gets the UUID
   *
   * @return
   *    - The active configuration UUID
   */
  static const char *get_uuid() { return _uuid; }

  static int64_t get_period();

private:
  static std::vector<TimingConfig> _timing; /*!< list of operational hours */
  static std::vector<TimingConfig>::iterator _active;
  static char _uuid[40]; /*! config UUID */

  /**
   * @brief Gets the default active configuration
   *
   * @return The default active configuration
   */
  static TimingConfig get_default_active_config();
};