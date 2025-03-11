#pragma once

#include "driver/temperature_sensor.h"
#include "isensor.h"

class CpuTemp : public ISensor {
public:
  CpuTemp() = default;
  ~CpuTemp() {
    if (_temp_handle != nullptr) {
      temperature_sensor_uninstall(_temp_handle);
    }
  };

  esp_err_t init() override;
  esp_err_t read() override;
  float get_value() const override;

private:
  float _cpu_temp = 0.0f;
  temperature_sensor_handle_t _temp_handle = nullptr;
};