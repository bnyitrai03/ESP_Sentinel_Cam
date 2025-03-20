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
   * @brief Loads the dynamic configuration from a JSON document
   *
   * @param config The new configuration as a JSON document
   *
   */
  static void load_config(JsonDocument &config);

  /**
   * @brief Loads the dynamic configuration from the NVS storage, also handles
   * validation
   *
   * @note This function will restart the device if the configuration is invalid
   *
   */
  static void load_from_storage();

  /**
   * @brief Validates the configuration
   *
   * @note The configuration is valid if:
   *
   *   - The UUID is not empty
   *
   *   - The UUID is a string
   *
   *   - The UUID is not too long
   *
   *   - The timing array is not missing
   *
   *   - The period is an integer and not less than -1
   *
   *   - The start and end times are in the format HH:MM:SS
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
   * @return -1 if the active config period is -1 (sleeping), else 0
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

  /**
   * @brief Gets the period of the active configuration
   *
   * @return
   *    - The active configuration period
   */
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