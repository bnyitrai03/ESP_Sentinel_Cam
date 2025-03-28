#include "mqtt.h"
#include "storage.h"
#include "unity.h"
#include <ArduinoJson.h>

class MQTTTestHelper {
public:
  static void call_handle_header_ack_message(const char *ack_msg,
                                             uint32_t len) {
    MQTT::handle_header_ack_message(ack_msg, len);
  }

  static void call_handle_new_config(JsonDocument &doc) {
    MQTT::handle_new_config(doc);
  }
};

static MQTT *test_mqtt = nullptr;

void test_write_mqtt_static_config() {
  Storage::write("mqttAddress", "mqtt://test.mqtt.com");
  Storage::write("mqttUser", "test_user");
  Storage::write("mqttPassword", "test_password");
  Storage::write("configTopic", "test/config");
  Storage::write("healthRepTopic", "test/health_report");
  Storage::write("imageAckTopic", "test/image_ack");
  Storage::write("logTopic", "test/log_topic");
  Storage::write("imageTopic", "test/image_topic");
}

JsonDocument test_config() {
  JsonDocument doc;
  const char *test_config = R"(
    {
        "configId": "8D8AC610-566D-4EF0-9C22-186B",
        "timing": [
            {
                "period": -1,
                "start": "00:00:00",
                "end": "07:00:00"
            }
        ]
    })";
  DeserializationError error = deserializeJson(doc, test_config);
  TEST_ASSERT(error == DeserializationError::Ok);
  return doc;
}

TEST_CASE("Correct header acknowledgement received", "[mqtt]") {
  Storage storage;
  test_write_mqtt_static_config();
  test_mqtt = new MQTT();

  test_mqtt->wait_for_header_ack("2025-03-28T11:08:28Z", 1000);
  MQTTTestHelper::call_handle_header_ack_message(
      "2025-03-28T11:08:28Z", sizeof("2025-03-28T11:08:28Z"));

  delete test_mqtt;
  test_mqtt = nullptr;
}

TEST_CASE("Non-matching header acknowledgement received", "[mqtt]") {
  test_mqtt = new MQTT();

  TEST_ASSERT_TRUE(
      test_mqtt->wait_for_header_ack("2025-03-28T11:08:28Z", 1000));
  MQTTTestHelper::call_handle_header_ack_message("incorrect_timestamp",
                                                 sizeof("incorrect_timestamp"));

  delete test_mqtt;
  test_mqtt = nullptr;
}

TEST_CASE("Correct new configuration received and loaded", "[mqtt]") {
  test_mqtt = new MQTT();

  JsonDocument doc = test_config();
  MQTTTestHelper::call_handle_new_config(doc);

  delete test_mqtt;
  test_mqtt = nullptr;
}

TEST_CASE("Incorrect new configuration received", "[mqtt]") {
  test_mqtt = new MQTT();

  JsonDocument doc = test_config();
  doc["configId"] = nullptr;
  MQTTTestHelper::call_handle_new_config(doc);

  delete test_mqtt;
  test_mqtt = nullptr;
}