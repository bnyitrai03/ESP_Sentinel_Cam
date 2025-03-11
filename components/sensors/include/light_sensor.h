#pragma once

#include "myadc.h"

class LightSensor : public Adc {
public:
  LightSensor()
      : Adc(ADC_CHANNEL_1, ADC_UNIT_1, 0.0f) {
  } // ADC1_CHANNEL_1 = GPIO_NUM_2, 0V offset

protected:
  float convert_adc_reading(int raw_value) override;
};