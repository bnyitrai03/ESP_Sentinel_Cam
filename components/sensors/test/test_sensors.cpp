#include "sensors.h"
#include "unity.h"
#include <ArduinoJson.h>

TEST_CASE("Test reading sensor values", "[sensors]") {
  JsonDocument doc;
  Sensors sensors;

  esp_err_t err = sensors.init();
  TEST_ASSERT_EQUAL(ESP_OK, err);

  err = sensors.read_sensors(doc);
  TEST_ASSERT_EQUAL(ESP_OK, err);

  TEST_ASSERT(doc.containsKey("batteryCharge"));
  TEST_ASSERT(doc.containsKey("chargeCurrent"));
  TEST_ASSERT(doc.containsKey("batteryTemp"));
  TEST_ASSERT(doc.containsKey("cpuTemp"));
  TEST_ASSERT(doc.containsKey("luminosity"));
}
