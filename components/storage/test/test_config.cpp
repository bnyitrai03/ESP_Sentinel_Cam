#include "config.h"
#include "esp_log.h"
#include "unity.h"
#include <ArduinoJson.h>

const char *correct_test_config = R"(
    {
        "configId": "8D8AC610-566D-4EF0-9C22-186B",
        "timing": [
            {
                "period": -1,
                "start": "00:00:00",
                "end": "07:00:00"
            },
            {
                "period": 30,
                "start": "07:00:00",
                "end": "12:00:00"
            },
            {
                "period": 40,
                "start": "12:00:00",
                "end": "15:00:00"
            },
            {
                "period": 30,
                "start": "15:00:00",
                "end": "17:00:00"
            },
            {
                "period": 40,
                "start": "17:00:00",
                "end": "22:00:00"
            },
            {
                "period": -1,
                "start": "22:00:00",
                "end": "23:59:59"
            }
        ]
    })";

JsonDocument deserialize_config() {
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, correct_test_config);
  TEST_ASSERT(error == DeserializationError::Ok);
  return doc;
}

TEST_CASE("Validate incorrect config", "[config]") {
  {
    JsonDocument doc = deserialize_config();
    doc["configId"] = "";
    TEST_ASSERT_FALSE_MESSAGE(Config::validate(doc),
                              "Empty UUID should fail validation");
  }

  {
    JsonDocument doc = deserialize_config();
    doc["configId"] = 12345;
    TEST_ASSERT_FALSE_MESSAGE(Config::validate(doc),
                              "Non-string UUID should fail validation");
  }

  {
    JsonDocument doc = deserialize_config();
    doc["configId"] = "8D8AC610-566D-4EF0-9C22-186B-8D8AC610-566D-4EF0-9C22-"
                      "186B-8D8AC610-566D-4EF0-9C22-186B";
    TEST_ASSERT_FALSE_MESSAGE(Config::validate(doc),
                              "Too long UUID should fail validation");
  }

  {
    JsonDocument doc = deserialize_config();
    doc["timing"] = nullptr;
    TEST_ASSERT_FALSE_MESSAGE(Config::validate(doc),
                              "Missing timing array should fail validation");
  }

  {
    JsonDocument doc = deserialize_config();
    doc["timing"][0]["start"] = "25:90:00";
    TEST_ASSERT_FALSE_MESSAGE(Config::validate(doc),
                              "Invalid time format should fail validation");
  }

  {
    JsonDocument doc = deserialize_config();
    doc["timing"][0]["period"] = -2;
    TEST_ASSERT_FALSE_MESSAGE(
        Config::validate(doc),
        "Negative period (other than -1) should fail validation");
  }
}

TEST_CASE("Get default active config", "[config]") {
  TimingConfig tc = Config::get_active_config();

  TEST_ASSERT_EQUAL_INT32(40, tc.period);

  TEST_ASSERT_EQUAL_INT32(0, tc.start.get_hours());
  TEST_ASSERT_EQUAL_INT32(0, tc.start.get_minutes());
  TEST_ASSERT_EQUAL_INT32(0, tc.start.get_seconds());

  TEST_ASSERT_EQUAL_INT32(23, tc.end.get_hours());
  TEST_ASSERT_EQUAL_INT32(59, tc.end.get_minutes());
  TEST_ASSERT_EQUAL_INT32(59, tc.end.get_seconds());
}

TEST_CASE("Set correct active config", "[config]") {
  JsonDocument doc = deserialize_config();
  TEST_ASSERT(Config::validate(doc));

  Config::load_config(doc);
  TEST_ASSERT_EQUAL_INT32(-1, Config::set_active_config());
}
