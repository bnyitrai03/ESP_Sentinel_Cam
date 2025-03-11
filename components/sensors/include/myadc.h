#pragma once

#include "esp_adc/adc_oneshot.h"
#include "isensor.h"

class Adc : public ISensor {
public:
  Adc(adc_channel_t channel, adc_unit_t unit = ADC_UNIT_1, float offset = 0.0f);
  virtual ~Adc();

  esp_err_t init() override;
  esp_err_t read() override;
  float get_value() const override;

protected:
  virtual float convert_adc_reading(int raw_value) = 0;

  adc_channel_t _channel;
  adc_unit_t _unit;
  float _offset;
  float _value = 0;
  const float ADC_MAX = 4095.0f; // 12-bit ADC
  const float V_MAX = 3.1f;      // Reference voltage

private:
  static adc_oneshot_unit_handle_t _adc_handle;
  static int _init_count;
};