#pragma once
#include <esp_err.h>

class ISensor {
public:
  virtual ~ISensor() = default;
  virtual esp_err_t init() = 0;
  virtual esp_err_t read() = 0;
  virtual float get_value() const = 0;
};