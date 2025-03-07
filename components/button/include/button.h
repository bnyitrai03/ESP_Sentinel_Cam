#pragma once

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

class Button {
public:
  Button();
  ~Button();

private:
  static void button_task(void *arg);
  static void IRAM_ATTR gpio_isr_handler(void *arg);

  const gpio_num_t button_pin = GPIO_NUM_48;
  static constexpr uint32_t DEBOUNCE_TIME_MS = 50;
  QueueHandle_t event_queue = nullptr;
};