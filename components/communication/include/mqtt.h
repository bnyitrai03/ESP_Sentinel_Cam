#include "mqtt_client.h"
#include <string>

#define TIMESTAMP_SIZE 21
#define NAME_SIZE 64

/**
 * @brief Manages MQTT connections and messaging
 */
class MQTT {
public:
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
   */
  void publish(const char *topic, const char *data, uint32_t len);

  /**
   * @brief Waits for an acknowledgment message with a specific timestamp
   *
   * @param expected_timestamp The expected timestamp for the acknowledgment
   * @return
   *     - true : if the acknowledgment with the expected timestamp is received
   *
   *     - false : if the acknowledgment is not received within the timeout
   *
   */
  bool wait_for_sendack(const char *expected_timestamp);

  /**
   * @brief Handles remote logging
   *
   * @param fmt The format string for the log message
   * @param args The arguments for the format string
   * @return The size of the log message
   *
   */
  static int remote_log_handler(const char *fmt, va_list args);

private:
  /**
   * @brief Event handler registered to receive MQTT events
   *
   * This function is called by the MQTT client event loop.
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
   * @brief Handles acknowledgment messages
   *
   * @param topic The topic of the acknowledgment message
   * @param data The data of the acknowledgment message
   * @param len The length of the acknowledgment message data
   *
   */
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