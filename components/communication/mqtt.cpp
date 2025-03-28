#include "mqtt.h"
#include "config.h"
#include "error_handler.h"
#include "esp_log.h"
#include "storage.h"

constexpr auto *TAG = "MQTT";

esp_mqtt_client_config_t MQTT::_config;
esp_mqtt_client_handle_t MQTT::_client;
char MQTT::_uri[NAME_SIZE] = {0};
char MQTT::_username[NAME_SIZE] = {0};
char MQTT::_password[NAME_SIZE] = {0};
char MQTT::_config_topic[NAME_SIZE] = {0};
char MQTT::_health_report_topic[NAME_SIZE] = {0};
char MQTT::_imageack_topic[NAME_SIZE] = {0};
char MQTT::_log_topic[NAME_SIZE] = {0};
char MQTT::_image_topic[NAME_SIZE] = {0};
bool MQTT::_new_config_received = false;
int MQTT::_qos = 2;
int MQTT::_error_count = 0;
char MQTT::_expected_timestamp[TIMESTAMP_SIZE];
SemaphoreHandle_t MQTT::_ack_header_semaphore = xSemaphoreCreateBinary();
SemaphoreHandle_t MQTT::_config_semaphore = xSemaphoreCreateBinary();
bool MQTT::_connected = false;

MQTT::MQTT() {
  set_mqtt_deinit_callback([]() {
    _connected = false;
    // Disable the MQTT log handler
    esp_log_set_vprintf(vprintf);
    esp_mqtt_client_destroy(_client);
  });

  // Disable lib logging else remote logging dies
  esp_log_level_set("mqtt5_client", ESP_LOG_NONE);
  esp_log_level_set("mqtt_client", ESP_LOG_NONE);

  // -------------------- MQTT static config ----------------------------------
  if (Storage::read("mqttAddress", _uri, sizeof(_uri)) != ESP_OK ||
      Storage::read("mqttUser", _username, sizeof(_username)) != ESP_OK ||
      Storage::read("mqttPassword", _password, sizeof(_password)) != ESP_OK) {
    ESP_LOGE(TAG, "Failed to read MQTT credentials from storage!");
    restart();
  }

  if (Storage::read("configTopic", _config_topic, sizeof(_config_topic)) !=
          ESP_OK ||
      Storage::read("healthRepTopic", _health_report_topic,
                    sizeof(_health_report_topic)) != ESP_OK ||
      Storage::read("imageAckTopic", _imageack_topic,
                    sizeof(_imageack_topic)) != ESP_OK ||
      Storage::read("logTopic", _log_topic, sizeof(_log_topic)) != ESP_OK ||
      Storage::read("imageTopic", _image_topic, sizeof(_image_topic)) !=
          ESP_OK) {
    ESP_LOGE(TAG, "Failed to read MQTT topics from storage!");
    restart();
  }
  // -------------------------------------------------------------------------

  _config = {
      .broker =
          {
              .address =
                  {
                      .uri = _uri,
                  },
          },
      .credentials = {.username = _username,
                      .authentication =
                          {
                              .password = _password,
                          }},
      .session =
          {
              .protocol_ver = MQTT_PROTOCOL_V_5,
          },
  };
}

int MQTT::remote_log_handler(const char *fmt, va_list args) {
  // logs to the serial output
  vprintf(fmt, args);
  // assembles the remote log message
  char logBuff[LOG_SIZE] = {0};
  int size = vsnprintf(logBuff, LOG_SIZE, fmt, args);
  esp_mqtt_client_publish(_client, _log_topic, logBuff, size, _qos, 0);
  return size;
}

void MQTT::event_handler(void *handler_args, esp_event_base_t base,
                         int32_t event_id, void *event_data) {
  esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;
  switch (event_id) {
  case MQTT_EVENT_CONNECTED:
    // Enable the MQTT log handler
    esp_log_set_vprintf(remote_log_handler);
    // Subscribe to topics
    subscribe(_imageack_topic);
    subscribe(_config_topic);
    break;
  case MQTT_EVENT_DISCONNECTED:
    if (_connected) {
      esp_mqtt_client_reconnect(_client);
      vTaskDelay(pdMS_TO_TICKS(100));
    }
    break;
  case MQTT_EVENT_SUBSCRIBED:
    break;
  case MQTT_EVENT_UNSUBSCRIBED:
    break;
  case MQTT_EVENT_PUBLISHED:
    break;
  case MQTT_EVENT_DATA:
    if (strncmp(event->topic, _imageack_topic, event->topic_len) == 0) {
      handle_header_ack_message(event->data, event->data_len);
    }

    if (strncmp(event->topic, _config_topic, event->topic_len) == 0) {
      JsonDocument config;
      DeserializationError error =
          deserializeJson(config, event->data, event->data_len);
      if (error) {
        ESP_LOGE(TAG, "Error receiving the new config");
        ESP_LOGE(TAG, "Error parsing JSON: %s", error.c_str());
        return;
      }

      if (config.as<std::string>() == "config-ok") {
        _new_config_received = false;
        xSemaphoreGive(_config_semaphore);
        ESP_LOGI(TAG, "Received config-ok message!");
      } else {
        handle_new_config(config);
      }
    }
    break;
  case MQTT_EVENT_ERROR:
    // Disable the MQTT log handler
    esp_log_set_vprintf(vprintf);
    ESP_LOGE(TAG, "Error during MQTT communication!");
    ESP_LOGE(TAG, "Error type: %d", event->error_handle->error_type);
    _error_count++;
    ESP_LOGE(TAG, "MQTT communication error count: %d", _error_count);
    if (_error_count > 20) {
      ESP_LOGE(TAG, "Too many MQTT communication errors!");
      restart();
    }
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

esp_err_t MQTT::publish(const char *topic, const char *data, uint32_t len) {
  uint32_t ret =
      esp_mqtt_client_publish(_client, topic, data, len, _qos, false);
  if (ret == -1 || ret == -2) {
    return ESP_FAIL;
  } else {
    return ESP_OK;
  }
}

void MQTT::subscribe(const char *topic) {
  esp_mqtt_client_subscribe(_client, topic, _qos);
}

bool MQTT::wait_for_header_ack(const char *timestamp, uint32_t timeout) {
  snprintf(_expected_timestamp, sizeof(_expected_timestamp), "%s", timestamp);
  return xSemaphoreTake(_ack_header_semaphore, pdMS_TO_TICKS(timeout)) ==
         pdTRUE;
}

void MQTT::handle_header_ack_message(const char *ack_msg, uint32_t len) {
  if (len > TIMESTAMP_SIZE) {
    ESP_LOGE(TAG, "Received timestamp is too long!");
    return;
  }
  char received_timestamp[TIMESTAMP_SIZE] = {0};
  strncpy(received_timestamp, ack_msg, len);

  if (strcmp(received_timestamp, _expected_timestamp) == 0) {
    xSemaphoreGive(_ack_header_semaphore);
    ESP_LOGI(TAG, "Received matching acknowledgement timestamp: %s",
             received_timestamp);
  } else {
    ESP_LOGE(TAG, "Received non-matching timestamp!");
    ESP_LOGE(TAG, "Received timestamp: %s", received_timestamp);
    ESP_LOGE(TAG, "Expected timestamp: %s", _expected_timestamp);
  }
}

void MQTT::handle_new_config(JsonDocument &doc) {
  if (Config::validate(doc)) {
    std::string config;
    serializeJson(doc, config);

    esp_err_t err = Storage::write("dynamic_config", config);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Failed to write new config to storage!");
      restart();
    }

    Config::load_config(doc);
    ESP_LOGI(TAG, "New config loaded!");
    _new_config_received = true;
  } else {
    ESP_LOGE(TAG, "Invalid config received!");
  }
  xSemaphoreGive(_config_semaphore);
}

bool MQTT::wait_for_config(uint32_t timeout) {
  return xSemaphoreTake(_config_semaphore, pdMS_TO_TICKS(timeout)) == pdTRUE;
}