#pragma once

#include "esp_adc/adc_oneshot.h"
#include "isensor.h"

/**
 * @brief
 * ADC sensor class
 *
 */
class Adc : public ISensor {
public:
  Adc(adc_channel_t channel, adc_unit_t unit = ADC_UNIT_1, float offset = 0.0f);
  virtual ~Adc();

  /**
   * @brief Initializes the ADC handle and channel.
   */
  esp_err_t init() override;
  /**
   * @brief Reads the ADC value from the channel.
   */
  esp_err_t read() override;
  /**
   * @brief Returns the sensor value.
   */
  float get_value() const override;

protected:
  /**
   * @brief Converts the raw ADC reading to a sensor value.
   */
  virtual float convert_adc_reading(int raw_value) = 0;

  adc_channel_t _channel;        /*!< Channel */
  adc_unit_t _unit;              /*!< Unit */
  float _offset = 0.0f;          /*!< Offset */
  float _value = 0.0f;           /*!< Sensor value */
  const float ADC_MAX = 4095.0f; /*!< Maximum 12 bit ADC value */
  const float V_MAX = 3.1f;      /*!< Maximum voltage */

private:
  static adc_oneshot_unit_handle_t _adc_handle;
  static int _init_count; // Number of initialized ADC channels
};