#include "esp_event.h"
#include "freertos/event_groups.h"
#include <string>

// TODO: Implement robust error handling
class Wifi {
public:
  Wifi();
  ~Wifi();

  void init();
  void connect(const std::string &ssid, const std::string &password);

private:
  static void eventHandler(void *arg, esp_event_base_t event_base,
                           int32_t event_id, void *event_data);
  static EventGroupHandle_t _wifi_event_group;
  static const int WIFI_CONNECTED_BIT = BIT0;
  static bool _connected;
};