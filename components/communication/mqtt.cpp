#include "mqtt.h"
#include "error_handler.h"
#include "esp_log.h"

constexpr auto *TAG = "MQTT";

esp_mqtt_client_config_t MQTT::_config;
esp_mqtt_client_handle_t MQTT::_client;
char MQTT::_logBuff[256];
std::string MQTT::_configack_topic;
std::string MQTT::_health_report_topic;
std::string MQTT::_imageack_topic;
std::string MQTT::_log_topic;
std::string MQTT::_image_topic;
int MQTT::_qos;
char MQTT::_expected_timestamp[TIMESTAMP_SIZE];
SemaphoreHandle_t MQTT::_ack_semaphore = xSemaphoreCreateBinary();
bool MQTT::_connected = false;

MQTT::MQTT() {
  set_mqtt_deinit_callback([]() { esp_mqtt_client_destroy(_client); });
  // These should be read from NVS
  _config = {
      .broker =
          {
              .address =
                  {
                      .hostname = "192.168.0.232",
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
  _health_report_topic = "configrecv";
  _imageack_topic = "image_ack";
  _log_topic = "log";
  _image_topic = "image";
  _qos = 2;
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
    subscribe(_imageack_topic);
    break;
  case MQTT_EVENT_DISCONNECTED:
    if (_connected) {
      esp_mqtt_client_reconnect(_client);
    }
    break;
  case MQTT_EVENT_SUBSCRIBED:
    break;
  case MQTT_EVENT_UNSUBSCRIBED:
    break;
  case MQTT_EVENT_PUBLISHED:
    break;
  case MQTT_EVENT_DATA:
    if (strncmp(event->topic, _imageack_topic.c_str(), event->topic_len) == 0) {
      handle_sendack_message(event->topic, event->data, event->data_len);
    }
    break;
  case MQTT_EVENT_ERROR:
    break;
  }
}

void MQTT::start() {
  _client = esp_mqtt_client_init(&_config);
  esp_mqtt_client_register_event(_client, (esp_mqtt_event_id_t)ESP_EVENT_ANY_ID,
                                 event_handler, NULL);
  if (esp_mqtt_client_start(_client) != ESP_OK) {
    ESP_LOGE(TAG, "Failed to start MQTT client");
    restart();
  }
  _connected = true;
}

void MQTT::publish(const char *topic, const char *data, uint32_t len) {
  esp_mqtt_client_publish(_client, topic, data, len, _qos, false);
}

void MQTT::subscribe(const std::string &topic) {
  esp_mqtt_client_subscribe(_client, topic.c_str(), _qos);
}

bool MQTT::wait_for_sendack(const char *timestamp) {
  snprintf(_expected_timestamp, sizeof(_expected_timestamp), "%s", timestamp);
  return xSemaphoreTake(_ack_semaphore, pdMS_TO_TICKS(5000)) == pdTRUE;
}

void MQTT::handle_sendack_message(const char *topic, const char *data,
                                  uint32_t len) {
  char received_timestamp[TIMESTAMP_SIZE] = {0};
  strncpy(received_timestamp, data, sizeof(received_timestamp) - 1);

  if (strcmp(received_timestamp, _expected_timestamp) == 0) {
    xSemaphoreGive(_ack_semaphore);
    ESP_LOGI(TAG, "Received matching acknowledgement timestamp: %s",
             received_timestamp);
  } else {
    ESP_LOGE(TAG, "Received non-matching timestamp!");
    ESP_LOGE(TAG, "Received timestamp: %s", received_timestamp);
    ESP_LOGE(TAG, "Expected timestamp: %s", _expected_timestamp);
  }
}