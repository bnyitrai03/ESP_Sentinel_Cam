#pragma once

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <atomic>

/**
 * @brief Button class that handles button press and release events
 */
class Button {
public:
  /**
   * @brief State of the button
   */
  struct ButtonState {
    uint32_t press_start_time; /*!< When the button was pressed */
    int last_state;            /*!< Last state of the button */
    bool is_pressed;           /*!< Whether the button is pressed */
  };

  /**
   * @brief Install the button ISR and start the button task
   */
  Button();
  ~Button();

  /**
   * @brief Stop the button task
   */
  void stop();

private:
  /**
   * @brief Task that handles the button press and release events
   *
   * @param arg Button object
   */
  static void button_task(void *arg);
  /**
   * @brief ISR handler that sends the current button press time to the button
   * task
   *
   * @param arg Button object
   */
  static void IRAM_ATTR gpio_isr_handler(void *arg);

  /**
   * @brief Waits for a button event
   *
   * @param button Button object
   * @param current_time Pointer to the current time
   *
   * @return true if an event was received, false otherwise
   */
  static bool wait_for_button_event(Button *button, uint32_t *current_time);
  /**
   * @brief Decide whether the button was pressed or released
   *
   * @param button Button object
   * @param current_time The current time
   * @param state The button state
   */
  static void handle_button_state_change(Button *button, uint32_t current_time,
                                         ButtonState *state);
  /**
   * @brief Handle the button press event
   *
   * @param button Button object
   * @param current_time The current time
   * @param state The button state
   */
  static void handle_button_press(Button *button, uint32_t current_time,
                                  ButtonState *state);
  /**
   * @brief Publish an event to the event queue when the button is released
   *
   * @param button Button object
   * @param current_time The current time
   * @param state The button state
   */
  static void handle_button_release(Button *button, uint32_t current_time,
                                    ButtonState *state);

  TaskHandle_t _task_handle = nullptr;
  bool running = false;
  static constexpr uint32_t LONG_PRESS_TIME = pdMS_TO_TICKS(2500);
  const gpio_num_t button_pin = GPIO_NUM_48;
  QueueHandle_t event_queue = nullptr;
};