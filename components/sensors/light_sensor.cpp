#include "light_sensor.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include <cmath>

constexpr auto *TAG = "Light Sensor";

LightSensor::LightSensor(I2CManager &i2c)
    : _i2c(i2c), _device_handle(nullptr), _initialized(false) {}

LightSensor::~LightSensor() {
  if (_device_handle) {
    i2c_master_bus_rm_device(_device_handle);
  }
}

esp_err_t LightSensor::init() {
  esp_err_t err;

  i2c_device_config_t dev_cfg = {
      .dev_addr_length = I2C_ADDR_BIT_LEN_7,
      .device_address = I2C_DEVICE_ADDRESS_NOT_USED,
      .scl_speed_hz = 400000,
  };
  err = i2c_master_bus_add_device(_i2c.get_bus_handle(), &dev_cfg,
                                  &_device_handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Creating device failed: %s", esp_err_to_name(err));
    return err;
  }

  if (_i2c.probe(OPT3005_ADDRESS) != ESP_OK) {
    return ESP_ERR_NOT_FOUND;
  }

  if (this->configure_sensor() != ESP_OK) {
    ESP_LOGE(TAG, "Failed to configure sensor");
    return ESP_FAIL;
  }

  _initialized = true;
  return err;
}

esp_err_t LightSensor::read_register(uint8_t reg, uint16_t *value) {
  uint8_t write_address = (OPT3005_ADDRESS << 1);    // LSB = 0 for write
  uint8_t read_address = (OPT3005_ADDRESS << 1) | 1; // LSB = 1 for read

  uint8_t write_data[2] = {write_address, reg};
  uint8_t read_data[2] = {};
  *value = 0;

  i2c_operation_job_t read[] = {
      // write register address
      {.command = I2C_MASTER_CMD_START},
      {.command = I2C_MASTER_CMD_WRITE,
       .write = {.ack_check = true, .data = write_data, .total_bytes = 2}},
      {.command = I2C_MASTER_CMD_STOP},

      // send read command
      {.command = I2C_MASTER_CMD_START},
      {.command = I2C_MASTER_CMD_WRITE,
       .write = {.ack_check = true, .data = &read_address, .total_bytes = 1}},

      // read data
      {.command = I2C_MASTER_CMD_READ,
       .read = {.ack_value = I2C_ACK_VAL,
                .data = &read_data[0],
                .total_bytes = 1}},
      {.command = I2C_MASTER_CMD_READ,
       // after last byte there is NACK
       .read = {.ack_value = I2C_NACK_VAL,
                .data = &read_data[1],
                .total_bytes = 1}},
  };

  esp_err_t err =
      i2c_master_execute_defined_operations(_device_handle, read, 7, 1000);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to read register 0x%02X: %s", reg,
             esp_err_to_name(err));
    return err;
  }

  // Combine the two bytes into a single 16-bit value
  *value = (read_data[0] << 8) | read_data[1]; // big endian
  return err;
}

esp_err_t LightSensor::write_register(uint8_t reg, uint16_t value) {
  // Prepare the I2C data to write
  uint8_t address = OPT3005_ADDRESS << 1; // LSB = 0 for write
  uint8_t data_low = value & 0xFF;
  uint8_t data_high = (value >> 8) & 0xFF;
  uint8_t write_data[4] = {address, reg, data_high, data_low};

  // OPT3005 single register write operation sequence
  i2c_operation_job_t write[] = {
      {.command = I2C_MASTER_CMD_START},
      {.command = I2C_MASTER_CMD_WRITE,
       .write = {.ack_check = true, .data = write_data, .total_bytes = 4}},
      {.command = I2C_MASTER_CMD_STOP},
  };

  esp_err_t err =
      i2c_master_execute_defined_operations(_device_handle, write, 4, 1000);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to write register 0x%02X: %s", reg,
             esp_err_to_name(err));
  }
  return err;
}

esp_err_t LightSensor::configure_sensor() {
  uint16_t config;
  esp_err_t err = read_register(OPT3005_CONFIG_REG, &config);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to read config register: %s", esp_err_to_name(err));
    return err;
  }

  // Configure sensor for single shot mode with auto range and 100ms conversion
  config = (OPT3005_CONFIG_RANGE_AUTO | OPT3005_CONFIG_SINGLE_SHOT) &
           OPT3005_CONFIG_CONV_TIME_100MS;

  err = write_register(OPT3005_CONFIG_REG, config);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to write config values: %s", esp_err_to_name(err));
  }
  return err;
}

esp_err_t LightSensor::read() {
  if (!_initialized) {
    return ESP_FAIL;
  }

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
    vTaskDelay(pdMS_TO_TICKS(100)); // Max conversion time
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