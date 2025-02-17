#include <nvs_flash.h>
#include <string>

class Storage {
public:
  Storage();

  static esp_err_t write(const std::string &key, const std::string &value);
  static esp_err_t read(const std::string &key, char *value, uint32_t len);
};
