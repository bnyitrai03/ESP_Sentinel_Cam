#include "esp_event.h"
#include "freertos/event_groups.h"
#include <string>

class Wifi {
public:
  Wifi();
  ~Wifi();

  void init();
  void connect();

private:
  static void eventHandler(void *arg, esp_event_base_t event_base,
                           int32_t event_id, void *event_data);
  static EventGroupHandle_t _wifi_event_group;
  static const int WIFI_CONNECTED_BIT = BIT0;
  static bool _connected;
};