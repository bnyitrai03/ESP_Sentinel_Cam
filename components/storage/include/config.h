#include "mytime.h"
#include <vector>

typedef struct {
  int period; /*!< working period of the device, -1: sleeping */
  Time start; /*!< HH:MM:SS */
  Time end;   /*!< HH:MM:SS */
} TimingConfig;

class Config {
public:
  Config();

  /**
   * @brief Loads the configuration from the NVS storage
   * @note This function will restart the device if the configuration is invalid
   *
   */
  static void load();
  /**
   * @brief Updates the configuration
   *
   * @param config The new configuration as a string
   *
   */
  static void update(std::string &config);
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
   * @brief Gets the UUID
   *
   * @return
   *    - The active configuration UUID
   */
  static const char *get_uuid() { return uuid; }

private:
  static std::vector<TimingConfig> _timing; /*!< list of operational hours */
  static std::vector<TimingConfig>::iterator _active;
  static char uuid[40]; /*! active config UUID */

  /**
   * @brief Sets the active configuration based on the current time
   */
  void set_active_config();
  /**
   * @brief Gets the default active configuration
   *
   * @return The default active configuration
   */
  static TimingConfig get_default_active_config();
};