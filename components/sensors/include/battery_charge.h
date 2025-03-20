#pragma once

#include "driver/rtc_io.h"
#include "freertos/FreeRTOS.h"
#include "myadc.h"

class BatteryCharge : public Adc {
public:
  BatteryCharge() : Adc(ADC_CHANNEL_0, ADC_UNIT_1, 0.3f) {
    if (esp_reset_reason() == ESP_RST_DEEPSLEEP) {
      rtc_gpio_hold_dis(GPIO_NUM_1);
    }
  } // ADC1_CHANNEL_0 = GPIO_NUM_1, 0.3V offset

protected:
  /*
   * @brief
   * Converts the raw ADC reading to a battery percentage value.
   *
   * @param raw_value
   * Raw ADC reading
   *
   * @return
   * Battery percentage value
   *
   */
  float convert_adc_reading(int raw_value) override;
};