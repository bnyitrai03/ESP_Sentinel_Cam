#include "light_sensor.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include <cmath>

constexpr auto *TAG = "Light Sensor";

// TODO: might need to change measurement time to 100 ms

LightSensor::LightSensor(I2CManager &i2c)
    : _i2c(i2c), _device_handle(nullptr) {}

LightSensor::~LightSensor() {
  if (_device_handle) {
    i2c_master_bus_rm_device(_device_handle);
  }
}

esp_err_t LightSensor::init() {
  esp_err_t err;

  i2c_device_config_t dev_cfg = {
      .dev_addr_length = I2C_ADDR_BIT_LEN_7,
      .device_address = OPT3005_ADDRESS,
      .scl_speed_hz = 400000,
  };
  err = i2c_master_bus_add_device(_i2c.get_bus_handle(), &dev_cfg,
                                  &_device_handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Creating device failed: %s", esp_err_to_name(err));
    return err;
  }

  // Configure sensor for single shot mode with auto range and 800ms conversion
  uint16_t config = OPT3005_CONFIG_RANGE_AUTO | OPT3005_CONFIG_CONV_TIME_800MS |
                    OPT3005_CONFIG_SINGLE_SHOT;
  err = write_register(OPT3005_CONFIG_REG, config);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to configure sensor: %s", esp_err_to_name(err));
    return err;
  }

  _i2c.probe(OPT3005_ADDRESS);

  ESP_LOGI(TAG, "OPT3005 light sensor initialized successfully");
  return ESP_OK;
}

esp_err_t LightSensor::read_register(uint8_t reg, uint16_t *value) {
  esp_err_t err;
  uint8_t write_buf = reg;
  uint8_t read_buf[2];

  err = _i2c.write_and_read(_device_handle, &write_buf, 1, read_buf, 2);
  if (err != ESP_OK) {
    return err;
  }

  // OPT3005 uses MSB first format
  *value = (read_buf[0] << 8) | read_buf[1];
  return ESP_OK;
}

esp_err_t LightSensor::write_register(uint8_t reg, uint16_t value) {
  uint8_t write_buf[3];
  write_buf[0] = reg;
  write_buf[1] = (value >> 8) & 0xFF; // MSB
  write_buf[2] = value & 0xFF;        // LSB

  return _i2c.write(_device_handle, write_buf, 3);
}

esp_err_t LightSensor::read() {
  uint16_t result_reg;
  uint16_t config_reg;
  esp_err_t err;

  // Check if conversion is ready
  err = read_register(OPT3005_CONFIG_REG, &config_reg);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to read config register: %s", esp_err_to_name(err));
    return err;
  }

  if (!(config_reg & OPT3005_CONFIG_CONV_READY)) {
    ESP_LOGW(TAG, "Conversion not ready");
    vTaskDelay(pdMS_TO_TICKS(800)); // Max conversion time
  }

  err = read_register(OPT3005_RESULT_REG, &result_reg);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to read sensor value: %s", esp_err_to_name(err));
    return err;
  }

  if (config_reg & OPT3005_CONFIG_OVF) {
    ESP_LOGW(TAG, "Overflow detected in measurement");
  }

  uint8_t exponent = (result_reg >> 12) & 0x0F;
  uint16_t mantissa = result_reg & 0x0FFF;
  _light_value =
      0.02f * (1 << exponent) * mantissa; // 2 ^ exponent == 1 << exponent

  ESP_LOGI(TAG, "Light value: %.2f lux", _light_value);
  return ESP_OK;
}

float LightSensor::get_value() const { return std::round(_light_value); }