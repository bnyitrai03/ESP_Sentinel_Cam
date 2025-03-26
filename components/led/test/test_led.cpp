#include "esp_log.h"
#include "led.h"
#include "unity.h"

static Led *test_led = nullptr;

TEST_CASE("LED test", "[led]") {
  test_led = new Led();
  TEST_ASSERT_NOT_NULL(test_led);

  ESP_LOGI("LED test", "LED is off for 5 seconds");
  test_led->set_pattern(Led::Pattern::OFF);
  vTaskDelay(5000 / portTICK_PERIOD_MS);

  ESP_LOGI("LED test", "LED is on for 5 seconds");
  test_led->set_pattern(Led::Pattern::ON);
  vTaskDelay(5000 / portTICK_PERIOD_MS);

  ESP_LOGI("LED test", "NO_QR_CODE_BLINK blink for 5 seconds");
  test_led->set_pattern(Led::Pattern::NO_QR_CODE_BLINK);
  vTaskDelay(5000 / portTICK_PERIOD_MS);

  ESP_LOGI(
      "LED test",
      "STATIC_CONFIG_SAVED_BLINK and MQTT_CONNECTED_BLINK blink for 5 seconds");
  test_led->set_pattern(Led::Pattern::STATIC_CONFIG_SAVED_BLINK);
  vTaskDelay(5000 / portTICK_PERIOD_MS);

  ESP_LOGI("LED test", "ERROR_BLINK blink for 5 seconds");
  test_led->set_pattern(Led::Pattern::ERROR_BLINK);
  vTaskDelay(5000 / portTICK_PERIOD_MS);

  test_led->set_pattern(Led::Pattern::OFF);
  ESP_LOGW("LED test", "Stopped LED test");
  delete test_led;
  test_led = nullptr;
}