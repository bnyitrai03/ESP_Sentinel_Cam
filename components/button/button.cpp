#include "button.h"
#include "esp_log.h"
#include "esp_system.h"

constexpr auto *TAG = "Button";

Button::Button() {
  event_queue = xQueueCreate(1, sizeof(uint32_t));

  gpio_config_t io_conf = {.pin_bit_mask = (1ULL << button_pin),
                           .mode = GPIO_MODE_INPUT,
                           .pull_up_en = GPIO_PULLUP_ENABLE,
                           .pull_down_en = GPIO_PULLDOWN_DISABLE,
                           .intr_type = GPIO_INTR_ANYEDGE};
  gpio_config(&io_conf);

  // Install GPIO ISR service
  gpio_install_isr_service(0);
  // Add ISR handler
  if (gpio_isr_handler_add(button_pin, gpio_isr_handler, this) != ESP_OK) {
    ESP_LOGE(TAG, "Failed to add ISR handler");
    esp_restart();
  }

  auto res = xTaskCreate(button_task, "button_task", 4096, this, 20, nullptr);
  if (res != pdPASS) {
    ESP_LOGE(TAG, "Failed to create Button task");
    esp_restart();
  }
}

Button::~Button() {
  gpio_isr_handler_remove(button_pin);
  if (event_queue) {
    vQueueDelete(event_queue);
    event_queue = nullptr;
  }
  gpio_reset_pin(button_pin);
}

void IRAM_ATTR Button::gpio_isr_handler(void *arg) {
  Button *button = static_cast<Button *>(arg);
  uint32_t now = xTaskGetTickCountFromISR();
  xQueueSendFromISR(button->event_queue, &now, nullptr);
}

void Button::button_task(void *arg) {
  Button *button = static_cast<Button *>(arg);
  uint32_t last_press_time = 0;
  int last_state = 1;
  ESP_LOGI(TAG, "Button task started");

  while (true) {
    uint32_t current_time;
    // Task blocks here until event received
    if (xQueueReceive(button->event_queue, &current_time, portMAX_DELAY)) {
      if ((current_time - last_press_time) >= pdMS_TO_TICKS(DEBOUNCE_TIME_MS)) {
        int current_state = gpio_get_level(button->button_pin);
        if (current_state != last_state) {
          if (current_state == 0) {
            ESP_LOGI(TAG, "Button pressed on GPIO %d!", button->button_pin);
          }
          last_state = current_state;
          last_press_time = current_time;
        }
      }
    }
  }
}