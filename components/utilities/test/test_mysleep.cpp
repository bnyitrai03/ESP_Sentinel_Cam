#include "esp_sleep.h"
#include "esp_system.h"
#include "mysleep.h"
#include "unity.h"

void trigger_mysleep() { mysleep(3000000); }

void check_deepsleep_reset_reason() {
  esp_sleep_wakeup_cause_t wakeup = esp_sleep_get_wakeup_cause();
  esp_reset_reason_t reset = esp_reset_reason();

  TEST_ASSERT_EQUAL(ESP_RST_DEEPSLEEP, reset);
  TEST_ASSERT_EQUAL(ESP_SLEEP_WAKEUP_TIMER, wakeup);
}

TEST_CASE_MULTIPLE_STAGES("Test mysleep function with 3 seconds", "[mysleep]",
                          trigger_mysleep, check_deepsleep_reset_reason);