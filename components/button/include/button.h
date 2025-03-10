#pragma once

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <atomic>

class Button {
public:
  struct ButtonState {
    uint32_t last_press_time;
    uint32_t press_start_time;
    int last_state;
    bool is_pressed;
  };

  Button();
  ~Button();

  TaskHandle_t get_button_task_handle() { return _task_handle; }
  std::atomic<bool> &get_button_shutdown() { return _button_shutdown; }
  void register_shutdown_observer(TaskHandle_t task) { _observer_task = task; }

private:
  static void button_task(void *arg);
  static void IRAM_ATTR gpio_isr_handler(void *arg);

  static bool wait_for_button_event(Button *button, uint32_t *current_time);
  static bool is_debounced(uint32_t current_time, uint32_t last_press_time);
  static void handle_button_state_change(Button *button, uint32_t current_time,
                                         ButtonState *state);
  static void handle_button_press(Button *button, uint32_t current_time,
                                  ButtonState *state);
  static void handle_button_release(Button *button, uint32_t current_time,
                                    ButtonState *state);

  std::atomic<bool> _button_shutdown = false;
  TaskHandle_t _task_handle = nullptr;
  TaskHandle_t _observer_task = nullptr;

  static constexpr uint32_t LONG_PRESS_TIME = pdMS_TO_TICKS(2500);
  const gpio_num_t button_pin = GPIO_NUM_48;
  static constexpr uint32_t DEBOUNCE_TIME_MS = 50;
  QueueHandle_t event_queue = nullptr;
};