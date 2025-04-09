#include "mysleep.h"
#include "driver/rtc_io.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "led.h"

constexpr auto *TAG = "Sleep";

void mysleep(Time wake_up) {
  char timestamp[9] = {0};
  Time::get_time(timestamp, sizeof(timestamp));
  Time now(timestamp);

  int64_t sleep_time_us =
      wake_up.toSeconds() * 1000000 - now.toSeconds() * 1000000 - OVERHEAD;
  if (sleep_time_us < 500000) {
    ESP_LOGE(TAG, "Invalid sleep time: %lld us", sleep_time_us);
    esp_restart();
  } else {
    isolate_gpio();
    esp_sleep_enable_timer_wakeup(sleep_time_us);
    configure_button_wake_up();
    esp_deep_sleep_start();
  }
}

void mysleep(uint64_t period) {
  uint64_t elapsed_time = esp_timer_get_time();
  int64_t sleep_time_us = period * 1000000 - elapsed_time - OVERHEAD;

  if (sleep_time_us < 500000) {
    ESP_LOGE(TAG, "Invalid sleep time: %lld us", sleep_time_us);
    esp_restart();
  } else {
    ESP_LOGW(TAG, "Deep sleep %lld seconds", sleep_time_us / 1000000);
    isolate_gpio();
    esp_sleep_enable_timer_wakeup(sleep_time_us);
    configure_button_wake_up();
    esp_deep_sleep_start();
  }
}

void button_press_sleep() {
  isolate_gpio();

  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
  configure_button_wake_up();

  esp_deep_sleep_start();
}

void configure_button_wake_up() {
  // Configure GPIO21 as RTC IO for wakeup
  rtc_gpio_init(GPIO_NUM_21);
  rtc_gpio_set_direction(GPIO_NUM_21, RTC_GPIO_MODE_INPUT_ONLY);
  rtc_gpio_pullup_en(GPIO_NUM_21);
  rtc_gpio_pulldown_dis(GPIO_NUM_21);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_21, 0);
}

void isolate_gpio() {
  // Isolate Camera pins
  rtc_gpio_isolate(GPIO_NUM_14); // CAM_PIN_PWDN
  rtc_gpio_isolate(GPIO_NUM_7);  // CAM_PIN_XCLK
  rtc_gpio_isolate(GPIO_NUM_4);  // CAM_PIN_SIOD
  rtc_gpio_isolate(GPIO_NUM_5);  // CAM_PIN_SIOC

  rtc_gpio_isolate(GPIO_NUM_11); // CAM_PIN_D7
  rtc_gpio_isolate(GPIO_NUM_15); // CAM_PIN_D6
  rtc_gpio_isolate(GPIO_NUM_10); // CAM_PIN_D5
  rtc_gpio_isolate(GPIO_NUM_16); // CAM_PIN_D4
  rtc_gpio_isolate(GPIO_NUM_9);  // CAM_PIN_D3
  rtc_gpio_isolate(GPIO_NUM_17); // CAM_PIN_D2
  rtc_gpio_isolate(GPIO_NUM_8);  // CAM_PIN_D1
  rtc_gpio_isolate(GPIO_NUM_18); // CAM_PIN_D0

  rtc_gpio_isolate(GPIO_NUM_13); // CAM_PIN_VSYNC
  rtc_gpio_isolate(GPIO_NUM_6);  // CAM_PIN_HREF
  rtc_gpio_isolate(GPIO_NUM_12); // CAM_PIN_PCLK

  // Isolate LED pin
  gpio_set_level(LED_PIN, 0);
  gpio_hold_en(LED_PIN);
}