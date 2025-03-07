#include "led.h"
#include "driver/rtc_io.h"
#include "esp_log.h"
#include "esp_system.h"

constexpr auto *TAG = "LED";

bool Led::led_state = false;
Led::Pattern Led::current_pattern = Led::Pattern::OFF;
SemaphoreHandle_t Led::pattern_mutex = xSemaphoreCreateMutex();

Led::Led() {
  // Reset the LED state if the device is waking up from deep sleep
  if (esp_reset_reason() == ESP_RST_DEEPSLEEP) {
    rtc_gpio_hold_dis(LED_PIN);
  }

  gpio_config_t io_conf = {};
  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pin_bit_mask = (1ULL << LED_PIN);
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  gpio_config(&io_conf);
  gpio_set_level(LED_PIN, 0);

  running = true;
  auto res = xTaskCreate(led_task, "led_task", 2048, this, 2, nullptr);
  if (res != pdPASS) {
    ESP_LOGE(TAG, "Failed to create LED task");
    esp_restart();
  }
}

Led::~Led() {
  running = false;
  if (pattern_mutex != nullptr) {
    vSemaphoreDelete(pattern_mutex);
    pattern_mutex = nullptr;
  }
}

void Led::set_pattern(Pattern pattern) {
  if (xSemaphoreTake(pattern_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    current_pattern = pattern;
    xSemaphoreGive(pattern_mutex);
  }
}

void Led::led_task(void *arg) {
  // Give context to the task
  auto *led = static_cast<Led *>(arg);
  led->task_function();
  // Clean up
  vTaskDelete(nullptr);
}

void Led::task_function() {
  Pattern current_pattern_local;
  while (running) {
    if (xSemaphoreTake(pattern_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
      current_pattern_local = current_pattern;
      xSemaphoreGive(pattern_mutex);
    } else {
      ESP_LOGW(TAG, "Failed to acquire mutex for pattern displaying");
      vTaskDelay(pdMS_TO_TICKS(100));
      continue;
    }

    switch (current_pattern_local) {
    case Pattern::OFF:
      gpio_set_level(LED_PIN, 0);
      led_state = false;
      vTaskDelay(1000);
      break;

    case Pattern::ON:
      gpio_set_level(LED_PIN, 1);
      led_state = true;
      vTaskDelay(1000);
      break;

    case Pattern::NO_QR_CODE_BLINK:
      toggle_led();
      vTaskDelay(pdMS_TO_TICKS(1000));
      break;

    case Pattern::STATIC_CONFIG_SAVED_BLINK:
      toggle_led();
      vTaskDelay(pdMS_TO_TICKS(500));
      break;

    case Pattern::MQTT_CONNECTED_BLINK:
      toggle_led();
      vTaskDelay(pdMS_TO_TICKS(500));
      break;

    case Pattern::ERROR_BLINK:
      toggle_led();
      vTaskDelay(pdMS_TO_TICKS(250));
      break;
    }
  }
}

void Led::toggle_led() {
  led_state = !led_state;
  if (gpio_set_level(LED_PIN, led_state) != ESP_OK) {
    ESP_LOGE(TAG, "Failed to set LED state");
  }
}