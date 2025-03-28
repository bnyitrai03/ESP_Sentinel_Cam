#pragma once

#include "esp_err.h"
#include "mqtt_client.h"
#include <ArduinoJson.h>
#include <string>

constexpr int TIMESTAMP_SIZE{21};
constexpr int NAME_SIZE{64};
constexpr int LOG_SIZE{256};

/**
 * @brief Manages MQTT connections and messaging
 */
class MQTT {
public:
  /**
   * @brief Helper class to test private methods of the MQTT class
   *
   */
  friend class MQTTTestHelper;

  /**
   * Registers a callback function to be called when the MQTT client is
   * deinitialized. Read the static config values from the storage and
   * initialize the MQTT client.
   */
  MQTT();

  /**
   * @brief Starts the MQTT client
   *
   */
  void start();

  /**
   * @brief Publishes a message to a specified topic
   *
   * @param topic The topic to publish the message to
   * @param data The message data to publish
   * @param len The length of the message data
   *
   * @return
   * - ESP_OK: the message was published successfully
   *
   * - ESP_FAIL: the message failed to publish
   *
   */
  esp_err_t publish(const char *topic, const char *data, uint32_t len);

  /**
   * @brief Waits for an acknowledgment message with a
   * specific timestamp, which will be sent upon receiving the header message
   *
   * @note This function blocks until the acknowledgment message is received or
   * the timeout is reached
   *
   * @param expected_timestamp The expected timestamp for the acknowledgment
   * @param timeout The timeout in milliseconds
   * @return
   *     - true : if the acknowledgment with the expected timestamp is received
   *
   *     - false : if the acknowledgment is not received within the timeout
   *
   */
  bool wait_for_header_ack(const char *expected_timestamp, uint32_t timeout);

  /**
   * @brief Waits for a new configuration message
   *
   * @param timeout The timeout in milliseconds
   *
   * @note This function blocks until the configuration message is received
   *
   * @return
   *     - true : if the configuration message is received
   *
   *     - false : if the configuration message is not received within the
   * timeout
   *
   */
  bool wait_for_config(uint32_t timeout);

  /**
   * @brief Publishes the logs remotely to the MQTT broker, and locally to the
   * ESP Monitor
   *
   * @param fmt The format string for the log message
   * @param args The arguments for the format string
   * @return The size of the log message
   *
   */
  static int remote_log_handler(const char *fmt, va_list args);

  /**
   * @return The health report topic
   *
   */
  const char *get_health_report_topic() { return _health_report_topic; }

  /**
   * @return The image topic
   *
   */
  const char *get_image_topic() { return _image_topic; }

  /**
   * @return Whether a new configuration has been received
   *
   */
  bool get_new_config_received() { return _new_config_received; }

private:
  /**
   * @brief Event handler registered to receive MQTT events
   *
   * This function is called by the MQTT client event loop. It handles
   * subscribing, recconections, and errors. It also dispatches the header
   * acknowledgment message handler and new configuration message handler.
   *
   * @param handler_args User data registered to the event
   * @param base Event base for the handler (always MQTT Base in this example)
   * @param event_id The id for the received event
   * @param event_data The data for the event, esp_mqtt_event_handle_t
   *
   */
  static void event_handler(void *handler_args, esp_event_base_t base,
                            int32_t event_id, void *event_data);

  /**
   * @brief Subscribes to a specified topic
   *
   * @param topic The topic to subscribe to
   *
   */
  static void subscribe(const char *topic);

  /**
   * @brief Compares the received timestamp with the expected timestamp, if they
   * match it releases the header acknowledge semaphore
   *
   * @note This function is called when a header acknowledgment message is
   * received
   *
   * @param ack_msg The data of the acknowledgment message
   * @param len The length of the acknowledgment message data
   *
   */
  static void handle_header_ack_message(const char *ack_msg, uint32_t len);

  /**
   * @brief Validate, save and load the new configuration
   *
   * @note This function is called when a new configuration message is received
   *
   * @param doc The JSON document containing the new configuration
   *
   */
  static void handle_new_config(JsonDocument &doc);

  static esp_mqtt_client_config_t _config;
  static esp_mqtt_client_handle_t _client;
  static char _uri[NAME_SIZE];
  static char _username[NAME_SIZE];
  static char _password[NAME_SIZE];
  static char _config_topic[NAME_SIZE];
  static char _health_report_topic[NAME_SIZE];
  static char _image_topic[NAME_SIZE];
  static char _imageack_topic[NAME_SIZE];
  static char _log_topic[NAME_SIZE];
  static int _qos;
  static int _error_count;
  static bool _new_config_received;
  static char _expected_timestamp[TIMESTAMP_SIZE];
  static SemaphoreHandle_t _ack_header_semaphore;
  static SemaphoreHandle_t _config_semaphore;
  static bool _connected;
};