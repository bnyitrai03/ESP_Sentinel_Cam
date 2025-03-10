#include "button.h"
#include "driver/rtc_io.h"
#include "error_handler.h"
#include "esp_log.h"
#include "esp_system.h"
#include "mysleep.h"

constexpr auto *TAG = "Button";

Button::Button() {
  event_queue = xQueueCreate(1, sizeof(uint32_t));

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
      xTaskCreate(button_task, "button_task", 4096, this, 20, &_task_handle);
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
  gpio_reset_pin(button_pin);
}

void IRAM_ATTR Button::gpio_isr_handler(void *arg) {
  Button *button = static_cast<Button *>(arg);
  uint32_t now = xTaskGetTickCountFromISR();
  xQueueSendFromISR(button->event_queue, &now, nullptr);
}

void Button::button_task(void *arg) {
  Button *button = static_cast<Button *>(arg);
  ButtonState state = {.last_press_time = 0,
                       .press_start_time = 0,
                       .last_state = 1,
                       .is_pressed = false};

  ESP_LOGI(TAG, "Button task started");

  while (true) {
    uint32_t current_time;
    if (!wait_for_button_event(button, &current_time)) {
      continue;
    }

    if (!is_debounced(current_time, state.last_press_time)) {
      continue;
    }

    handle_button_state_change(button, current_time, &state);
  }
}

bool Button::wait_for_button_event(Button *button, uint32_t *current_time) {
  return xQueueReceive(button->event_queue, current_time, portMAX_DELAY) ==
         pdTRUE;
}

bool Button::is_debounced(uint32_t current_time, uint32_t last_press_time) {
  return (current_time - last_press_time) >= pdMS_TO_TICKS(DEBOUNCE_TIME_MS);
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
  state->last_press_time = current_time;
}

void Button::handle_button_press(Button *button, uint32_t current_time,
                                 ButtonState *state) {
  ESP_LOGI(TAG, "Button pressed on GPIO %d!", button->button_pin);
  state->press_start_time = current_time;
  state->is_pressed = true;
  button->_button_shutdown = true;
  // Notify the observer task if registered
  if (button->_observer_task != nullptr) {
    xTaskNotify(button->_observer_task, 0, eNoAction);
  }
}

void Button::handle_button_release(Button *button, uint32_t current_time,
                                   ButtonState *state) {
  if (!state->is_pressed) {
    return;
  }

  uint32_t press_duration = current_time - state->press_start_time;

  // Disable the button interrupt to not disturb the shutdown sequence
  gpio_intr_disable(button->button_pin);
  gpio_isr_handler_remove(button->button_pin);

  if (press_duration >= LONG_PRESS_TIME) {
    ESP_LOGW(TAG, "Long press detected - Resetting device");
    reset_device();
  } else {
    ESP_LOGW(TAG, "Short press detected - Entering deep sleep");
    deinit_components();
    button_press_sleep();
  }
}