#pragma once

#include "mytime.h"

constexpr uint64_t BOOT_TIME_US = 1000000;    // ~1s for boot
constexpr uint64_t SHUTDOWN_TIME_US = 100000; // ~100ms for entering sleep
constexpr uint64_t WAKEUP_DELAY_US =
    2000; // CONFIG_ESP32S3_DEEP_SLEEP_WAKEUP_DELAY
constexpr uint64_t FLASH_READY_DELAY_US =
    2000; // CONFIG_ESP_SLEEP_WAIT_FLASH_READY_EXTRA_DELAY

constexpr uint64_t OVERHEAD =
    BOOT_TIME_US + SHUTDOWN_TIME_US + WAKEUP_DELAY_US + FLASH_READY_DELAY_US;

/**
 * @defgroup sleep_time Time-based sleep function
 * @{
 *
 * @brief Puts the device to sleep until the specified time.
 *
 * @param wake_up The time to wake up at in HH:MM:SS format.
 *
 * @note This function does not return.
 *
 */
void mysleep(Time wake_up);
/** @} */

/**
 * @defgroup sleep_time Time-based sleep function
 * @{
 *
 * @brief Puts the device to sleep until the next period.
 *
 * @param period The active period from the static config in seconds.
 *
 * @note This function does not return.
 *
 */
void mysleep(uint64_t period);
/** @} */

/**
 * @brief Puts the device to sleep until the button is pressed again.
 *
 * @note This function does not return.
 *
 */
void button_press_sleep();

/**
 * @brief Isolates the GPIO
 *
 * This function isolates the GPIO pins to minimize power consumption during
 * deep sleep.
 */
void isolate_gpio();