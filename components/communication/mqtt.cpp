#include "mqtt.h"
#include "esp_log.h"

constexpr auto *TAG = "MQTT";

esp_mqtt_client_config_t MQTT::_config;
esp_mqtt_client_handle_t MQTT::_client;
char MQTT::_logBuff[256];
std::string MQTT::_configack_topic;
std::string MQTT::_configrecv_topic;
std::string MQTT::_log_topic;
std::string MQTT::_image_topic;
int MQTT::_qos;

MQTT::MQTT() {
  // These should be read from NVS
  _config = {
      .broker =
          {
              .address =
                  {
                      .hostname = "192.168.68.108",
                      .transport = MQTT_TRANSPORT_OVER_TCP,
                      .port = 1883,
                  },
          },
      .credentials = {.username = "user",
                      .authentication =
                          {
                              .password = "password",
                          }},
      .session =
          {
              // might help with status messages
              .last_will =
                  {
                      .topic = nullptr,
                      .msg = nullptr,
                      .msg_len = 0,
                      .qos = 0,
                      .retain = false,
                  },
              .protocol_ver = MQTT_PROTOCOL_V_5,
          },
  };

  // also topics and qos ??
  _configack_topic = "configack";
  _configrecv_topic = "configrecv";
  _log_topic = "log";
  _image_topic = "mqtt/rpi/image";
  _qos = 2;
}

void MQTT::log_error_if_nonzero(const char *message, int error_code) {
  if (error_code != 0) {
    ESP_LOGE(TAG, "%s: %d", message, error_code);
  }
}

int MQTT::remote_log_handler(const char *fmt, va_list args) {
  // logs to the serial output
  vprintf(fmt, args);
  // assembles the remote log message
  int size = vsnprintf(_logBuff, 256, fmt, args);
  esp_mqtt_client_publish(_client, _log_topic.c_str(), _logBuff, size, _qos, 0);
  return size;
}

void MQTT::event_handler(void *handler_args, esp_event_base_t base,
                         int32_t event_id, void *event_data) {
  esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
  switch (event_id) {
  case MQTT_EVENT_CONNECTED:
    esp_log_set_vprintf(remote_log_handler);
    break;
  case MQTT_EVENT_DISCONNECTED:
    break;
  case MQTT_EVENT_SUBSCRIBED:
    break;
  case MQTT_EVENT_UNSUBSCRIBED:
    break;
  case MQTT_EVENT_PUBLISHED:
    break;
  case MQTT_EVENT_DATA:
    break;
  case MQTT_EVENT_ERROR:
    break;
  }
}

void MQTT::start() {
  _client = esp_mqtt_client_init(&_config);
  esp_mqtt_client_register_event(_client, (esp_mqtt_event_id_t)ESP_EVENT_ANY_ID,
                                 event_handler, NULL);
  esp_mqtt_client_start(_client);
}

void MQTT::publish(const char *topic, const char *data, size_t len) {
  esp_mqtt_client_publish(_client, topic, data, len, _qos, false);
}

void MQTT::subscribe(const std::string &topic) {
  esp_mqtt_client_subscribe(_client, topic.c_str(), _qos);
}