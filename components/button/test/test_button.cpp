#include "button.h"
#include "esp_log.h"
#include "unity.h"

static Button *test_button = nullptr;

TEST_CASE("Button test (short/long press)", "[button]") {
  test_button = new Button();
  TEST_ASSERT_NOT_NULL(test_button);

  ESP_LOGI("Button test", "Waiting 10 seconds for button press");
  vTaskDelay(10000 / portTICK_PERIOD_MS);
  ESP_LOGW("Button test", "Stopped button test");

  delete test_button;
  test_button = nullptr;
}