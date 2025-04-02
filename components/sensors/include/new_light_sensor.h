#pragma once

#include "driver/rtc_io.h"
#include "i2c_manager.h"
#include "isensor.h"

class NewLightSensor : public ISensor {
public:
  NewLightSensor(I2CManager &_i2c);
  ~NewLightSensor();

  esp_err_t init() override;
  esp_err_t read() override;
  float get_value() const override;

private:
  esp_err_t read_register(uint8_t reg, uint16_t *value);
  esp_err_t write_register(uint8_t reg, uint16_t value);

  I2CManager &_i2c;
  i2c_master_dev_handle_t _device_handle;
  float _light_value = -1.0f;

  // OPT3005 register addresses
  const uint16_t OPT3005_RESULT_REG = 0x00;
  const uint16_t OPT3005_CONFIG_REG = 0x01;

  // OPT3005 configuration bits
  const uint16_t OPT3005_CONFIG_RANGE_AUTO = 0xC000;      // Bits 15:12
  const uint16_t OPT3005_CONFIG_CONV_TIME_800MS = 0x0800; // Bit 11
  const uint16_t OPT3005_CONFIG_SINGLE_SHOT = 0x0100;     // Bits 10:9
  const uint16_t OPT3005_CONFIG_OVF = 0x0100;             // Bit 8
  const uint16_t OPT3005_CONFIG_CONV_READY = 0x0080;      // Bit 7
};