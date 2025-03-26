#pragma once

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

constexpr gpio_num_t LED_PIN = GPIO_NUM_21;

/**
 * @brief
 * Led class is responsible for controlling the LED on the device.
 *
 */
class Led {
public:
  /**
   * @brief
   * The pattern of the LED blinking
   *
   */
  enum class Pattern {
    OFF,
    ON,
    NO_QR_CODE_BLINK,          /*!< 1Hz = 1000 ms */
    STATIC_CONFIG_SAVED_BLINK, /*!< 2Hz = 500 ms */
    MQTT_CONNECTED_BLINK,      /*!< 2Hz = 500 ms */
    ERROR_BLINK                /*!< 4Hz = 250 ms */
  };

  /**
   * @brief Init the LED and start the task
   */
  Led();
  /**
   * @brief Stop the LED task
   */
  ~Led();

  /**
   * @brief Stop the LED task
   */
  void stop();
  /**
   * Set the blinking pattern of the LED
   * @param pattern The pattern to set
   */
  static void set_pattern(Pattern pattern);

private:
  /**
   * @brief Clean up after the LED task
   */
  static void led_task(void *arg);
  /**
   * @brief The task function that controls the LED
   */
  void task_function();
  /**
   * @brief Toggle the LED state
   */
  void toggle_led();

  static Pattern current_pattern;
  static SemaphoreHandle_t pattern_mutex;
  static bool led_state;
  bool running = false;
};