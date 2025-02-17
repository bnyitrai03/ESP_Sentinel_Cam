#include "mqtt_client.h"
#include <string>

#define TIMESTAMP_SIZE 21
#define NAME_SIZE 64

class MQTT {
public:
  MQTT();

  void start();
  void publish(const char *topic, const char *data, uint32_t len);
  bool wait_for_sendack(const char *expected_timestamp);
  static int remote_log_handler(const char *fmt, va_list args);

private:
  /*
   * @brief Event handler registered to receive MQTT events
   *
   *  This function is called by the MQTT client event loop.
   *
   * @param handler_args user data registered to the event.
   * @param base Event base for the handler(always MQTT Base in this example).
   * @param event_id The id for the received event.
   * @param event_data The data for the event, esp_mqtt_event_handle_t.
   */
  static void event_handler(void *handler_args, esp_event_base_t base,
                            int32_t event_id, void *event_data);
  static void subscribe(const char *topic);
  static void handle_sendack_message(const char *topic, const char *data,
                                     uint32_t len);

  static esp_mqtt_client_config_t _config;
  static esp_mqtt_client_handle_t _client;
  static char _hostname[NAME_SIZE];
  static char _username[NAME_SIZE];
  static char _password[NAME_SIZE];

  static char _configack_topic[NAME_SIZE];
  static char _health_report_topic[NAME_SIZE];
  static char _image_topic[NAME_SIZE];
  static char _imageack_topic[NAME_SIZE];
  static char _log_topic[NAME_SIZE];
  static char _logBuff[256];
  static int _qos;
  static char _expected_timestamp[TIMESTAMP_SIZE];
  static SemaphoreHandle_t _ack_semaphore;
  static bool _connected;
};