#pragma once

#include "mytime.h"

/**
 * @brief Puts the device to sleep until the specified time.
 *
 * @param wake_up The time to wake up at in HH:MM:SS format.
 *
 * @note This function does not return.
 *
 */
void mysleep(Time wake_up);

/**
 * @brief Puts the device to sleep until the next period.
 *
 * @note This function does not return.
 *
 */
void mysleep(uint64_t period);