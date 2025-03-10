#pragma once

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <functional>

constexpr gpio_num_t LED_PIN = GPIO_NUM_21;

class Led {
public:
  enum class Pattern {
    OFF,
    ON,
    NO_QR_CODE_BLINK,          /*!< 1Hz = 1000 ms */
    STATIC_CONFIG_SAVED_BLINK, /*!< 2Hz = 500 ms */
    MQTT_CONNECTED_BLINK,      /*!< 2Hz = 500 ms */
    ERROR_BLINK                /*!< 4Hz = 250 ms */
  };

  Led();
  ~Led();

  void stop();

  /*
   * Set the blinking pattern of the LED
   * @param pattern The pattern to set
   */
  static void set_pattern(Pattern pattern);

private:
  static void led_task(void *arg);
  void task_function();
  void toggle_led();

  static Pattern current_pattern;
  static SemaphoreHandle_t pattern_mutex;
  static bool led_state;
  bool running = false;
};