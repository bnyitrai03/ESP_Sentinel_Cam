#include "button.h"
#include "driver/rtc_io.h"
#include "error_handler.h"
#include "esp_log.h"
#include "esp_system.h"
#include "event_manager.h"
#include "mysleep.h"

constexpr auto *TAG = "Button";

Button::Button() {
  event_queue = xQueueCreate(1, sizeof(uint32_t));
  running = true;

  // TODO
  // rtc_gpio_deinit(button_pin); Doesn't work yet
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
    restart();
  }

  auto res =
      xTaskCreate(button_task, "button_task", 4096, this, 13, &_task_handle);
  if (res != pdPASS) {
    ESP_LOGE(TAG, "Failed to create Button task");
    restart();
  }
}

Button::~Button() {
  gpio_isr_handler_remove(button_pin);
  if (event_queue) {
    vQueueDelete(event_queue);
    event_queue = nullptr;
  }

  if (_task_handle != nullptr) {
    vTaskDelete(_task_handle);
    _task_handle = nullptr;
  }
}

void Button::stop() {
  if (_task_handle != nullptr) {
    eTaskState taskState = eTaskGetState(_task_handle);
    if (taskState != eDeleted && taskState != eInvalid) {
      vTaskSuspend(_task_handle);
      vTaskDelete(_task_handle);
      ESP_LOGI(TAG, "Button task deleted");
      _task_handle = nullptr;
    }
  }
}

void IRAM_ATTR Button::gpio_isr_handler(void *arg) {
  Button *button = static_cast<Button *>(arg);
  uint32_t now = xTaskGetTickCountFromISR();
  xQueueSendFromISR(button->event_queue, &now, nullptr);
}

void Button::button_task(void *arg) {
  Button *button = static_cast<Button *>(arg);
  ButtonState state = {
      .press_start_time = 0, .last_state = 1, .is_pressed = false};

  ESP_LOGI(TAG, "Button task started");

  while (button->running) {
    uint32_t current_time;
    wait_for_button_event(button, &current_time);
    handle_button_state_change(button, current_time, &state);
  }

  if (button->_task_handle != nullptr) {
    vTaskDelete(button->_task_handle);
    ESP_LOGI(TAG, "Button task deleted itself");
    button->_task_handle = nullptr;
  }
}

bool Button::wait_for_button_event(Button *button, uint32_t *current_time) {
  return xQueueReceive(button->event_queue, current_time, portMAX_DELAY) ==
         pdTRUE;
}

void Button::handle_button_state_change(Button *button, uint32_t current_time,
                                        ButtonState *state) {
  int current_state = gpio_get_level(button->button_pin);

  if (current_state == state->last_state) {
    return;
  }

  if (current_state == 0) { // Button pressed
    handle_button_press(button, current_time, state);
  } else { // Button released
    handle_button_release(button, current_time, state);
  }

  state->last_state = current_state;
}

void Button::handle_button_press(Button *button, uint32_t current_time,
                                 ButtonState *state) {
  ESP_LOGI(TAG, "Button pressed on GPIO %d!", button->button_pin);
  state->press_start_time = current_time;
  state->is_pressed = true;
  PUBLISH(EventType::BUTTON_PRESSED);
}

void Button::handle_button_release(Button *button, uint32_t current_time,
                                   ButtonState *state) {
  if (!state->is_pressed) {
    return;
  }

  uint32_t press_duration = current_time - state->press_start_time;

  if (press_duration >= LONG_PRESS_TIME) {
    ESP_LOGW(TAG, "Long press detected - Resetting device");
    PUBLISH(EventType::RESET);
  } else {
    ESP_LOGW(TAG, "Short press detected - Entering deep sleep");
    PUBLISH(EventType::SLEEP_UNTIL_BUTTON_PRESS);
  }
  button->running = false;
}