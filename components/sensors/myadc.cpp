#include "myadc.h"
#include "esp_log.h"

static const char *TAG = "ADC";

adc_oneshot_unit_handle_t Adc::_adc_handle = nullptr;
int Adc::_init_count = 0;

Adc::Adc(adc_channel_t channel, adc_unit_t unit, float offset)
    : _channel(channel), _unit(unit), _offset(offset) {}

Adc::~Adc() {
  _init_count--;
  if (_init_count <= 0 && _adc_handle != nullptr) {
    adc_oneshot_del_unit(_adc_handle);
    _adc_handle = nullptr;
  }
}

esp_err_t Adc::init() {
  esp_err_t ret = ESP_ERR_INVALID_STATE;
  if (_adc_handle == nullptr) {
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = _unit,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ret = adc_oneshot_new_unit(&init_config, &_adc_handle);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to initialize ADC unit: %s", esp_err_to_name(ret));
      return ret;
    }
  }

  adc_oneshot_chan_cfg_t config = {
      .atten = ADC_ATTEN_DB_12,
      .bitwidth = ADC_BITWIDTH_12,
  };

  ret = adc_oneshot_config_channel(_adc_handle, _channel, &config);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to configure ADC channel: %s", esp_err_to_name(ret));
    return ret;
  }

  _init_count++;
  return ESP_OK;
}

esp_err_t Adc::read() {
  if (_adc_handle == nullptr) {
    return ESP_ERR_INVALID_STATE;
  }

  int raw_value;
  esp_err_t ret = adc_oneshot_read(_adc_handle, _channel, &raw_value);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to read ADC value: %s", esp_err_to_name(ret));
    return ret;
  }

  _value = convert_adc_reading(raw_value);
  return ESP_OK;
}

float Adc::get_value() const { return _value; }