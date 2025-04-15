#pragma once

#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

constexpr gpio_num_t RED_PIN = GPIO_NUM_48;
constexpr gpio_num_t GREEN_PIN = GPIO_NUM_38;
constexpr gpio_num_t BLUE_PIN = GPIO_NUM_39;

class RGBLed {
public:
  /**
   * @brief
   * The pattern of the LED blinking
   *
   */
  enum class Pattern {
    OFF,
    ON,                  /*!< WHITE */
    NO_QR_CODE,          /*!< BLUE */
    STATIC_CONFIG_SAVED, /*!< GREEN */
    MQTT_CONNECTED,      /*!< ORANGE */
    ERROR_BLINK,         /*!< RED */
  };

  /**
   * @brief RGB color structure
   */
  struct RGBColor {
    uint8_t r;
    uint8_t g;
    uint8_t b;
  };

  /**
   * @brief Initialize the RGB LED and start the control task
   */
  RGBLed();

  /**
   * @brief Clean up and stop the LED task
   */
  ~RGBLed();

  /**
   * @brief Stop the RGB LED task
   */
  void stop();

  /**
   * @brief Set the blinking pattern of the RGB LED
   * @param pattern The pattern to set
   */
  static void set_pattern(Pattern pattern);

private:
  /**
   * @brief Static task function for FreeRTOS
   */
  static void rgb_led_task(void *arg);

  /**
   * @brief The task function that controls the RGB LED
   */
  void task_function();

  /**
   * @brief Set RGB values directly
   * @param r Red component (0-255)
   * @param g Green component (0-255)
   * @param b Blue component (0-255)
   */
  static void set_rgb(uint8_t r, uint8_t g, uint8_t b);

  /**
   * @brief Initialize the LED hardware
   * @return true if initialization successful, false otherwise
   */
  bool init_hardware();

  /**
   * @brief Set the LED to the color associated with the given pattern
   * @param pattern The pattern whose color to use
   */
  static void set_color(Pattern pattern);

  /**
   * @brief Toggle the LED between off and the color associated with the pattern
   * @param pattern The pattern whose color to use when turning on
   */
  static void blink_led(Pattern pattern);

  static const RGBColor PATTERN_COLORS[];

  static Pattern _current_pattern;
  static SemaphoreHandle_t _led_mutex;
  static SemaphoreHandle_t _pattern_mutex;
  bool _running = false;
  static bool _led_state;
};