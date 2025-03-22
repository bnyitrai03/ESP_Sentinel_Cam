#pragma once

#include "driver/rtc_io.h"
#include "freertos/FreeRTOS.h"
#include "myadc.h"

/**
 * @brief
 * Light sensor class
 *
 */
class LightSensor : public Adc {
public:
  LightSensor() : Adc(ADC_CHANNEL_1, ADC_UNIT_1, 0.0f) {
    if (esp_reset_reason() == ESP_RST_DEEPSLEEP) {
      rtc_gpio_hold_dis(GPIO_NUM_2);
    }
  } // ADC1_CHANNEL_1 = GPIO_NUM_2, 0V offset

protected:
  /**
   * @brief
   * Converts the raw ADC reading to a lux value.
   *
   * @param raw_value
   * Raw ADC reading
   *
   * @return
   * Lux value
   *
   */
  float convert_adc_reading(int raw_value) override;
};