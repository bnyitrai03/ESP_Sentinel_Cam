#include "mysleep.h"
#include "error_handler.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_timer.h"

constexpr auto *TAG = "Sleep";

constexpr uint64_t BOOT_TIME_US = 500000;     // ~500ms for boot
constexpr uint64_t SHUTDOWN_TIME_US = 100000; // ~100ms for entering sleep
constexpr uint64_t WAKEUP_DELAY_US =
    2000; // CONFIG_ESP32S3_DEEP_SLEEP_WAKEUP_DELAY
constexpr uint64_t FLASH_READY_DELAY_US =
    2000; // CONFIG_ESP_SLEEP_WAIT_FLASH_READY_EXTRA_DELAY

constexpr uint64_t OVERHEAD =
    BOOT_TIME_US + SHUTDOWN_TIME_US + WAKEUP_DELAY_US + FLASH_READY_DELAY_US;

void mysleep(Time wake_up) {
  char timestamp[9] = {0};
  Time::get_time(timestamp, sizeof(timestamp));
  Time now(timestamp);

  int64_t sleep_time_us =
      wake_up.toSeconds() * 1000000 - now.toSeconds() * 1000000 - OVERHEAD;
  if (sleep_time_us < 1000000) {
    ESP_LOGE(TAG, "Invalid sleep time: %lld us", sleep_time_us);
    restart();
  } else {
    ESP_LOGW(TAG, "Deep sleep until: %02d:%02d:%02d", wake_up.get_hours(),
             wake_up.get_minutes(), wake_up.get_seconds());
    esp_sleep_enable_timer_wakeup(sleep_time_us);
    esp_deep_sleep_start();
  }
}

void mysleep(uint64_t period) {
  uint64_t elapsed_time = esp_timer_get_time();
  int64_t sleep_time_us = period * 1000000 - elapsed_time - OVERHEAD;

  if (sleep_time_us < 1000000) {
    ESP_LOGE(TAG, "Invalid sleep time: %lld us", sleep_time_us);
    restart();
  } else {
    ESP_LOGW(TAG, "Deep sleep %llu seconds", sleep_time_us / 1000000);
    esp_sleep_enable_timer_wakeup(sleep_time_us);
    esp_deep_sleep_start();
  }
}