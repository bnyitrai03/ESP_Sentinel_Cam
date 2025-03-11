#pragma once

#include "esp_log.h"
#include "myadc.h"

class BatteryCharge : public Adc {
public:
  BatteryCharge()
      : Adc(ADC_CHANNEL_0, ADC_UNIT_1, 0.3f) {
  } // ADC1_CHANNEL_0 = GPIO_NUM_1, 0.3V offset

protected:
  float convert_adc_reading(int raw_value) override;
};