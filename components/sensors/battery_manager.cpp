#include "battery_manager.h"
#include "esp_log.h"

constexpr auto *TAG = "BatteryManager";

BatteryManager::BatteryManager(I2CManager &i2c)
    : _i2c(i2c), _initialized(false) {}

BatteryManager::~BatteryManager() {
  if (_initialized) {
    i2c_master_bus_rm_device(_device_handle);
  }
}

esp_err_t BatteryManager::init() {
  if (_initialized) {
    return ESP_OK;
  }

  i2c_master_bus_handle_t bus_handle = _i2c.get_bus_handle();
  i2c_device_config_t dev_cfg = {
      .dev_addr_length = I2C_ADDR_BIT_LEN_7,
      .device_address = BQ25622_ADDR,
      .scl_speed_hz = 400000,
  };

  esp_err_t err =
      i2c_master_bus_add_device(bus_handle, &dev_cfg, &_device_handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to add device to I2C bus: %d", err);
    return err;
  }

  // Probe device to check if it's connected
  _i2c.probe(BQ25622_ADDR);

  _initialized = true;
  ESP_LOGI(TAG, "BatteryManager initialized successfully");
  return ESP_OK;
}

esp_err_t BatteryManager::write_register(uint8_t reg, uint8_t data) {
  if (!_initialized) {
    return ESP_ERR_INVALID_STATE;
  }

  uint8_t write_buf[2] = {reg, data};
  return _i2c.write(_device_handle, write_buf, 2);
}

esp_err_t BatteryManager::read_register(uint8_t reg, uint16_t *data) {
  if (!_initialized) {
    return ESP_ERR_INVALID_STATE;
  }

  uint8_t read_buf[2];
  uint8_t write_buf = reg;
  esp_err_t err =
      _i2c.write_and_read(_device_handle, &write_buf, 1, read_buf, 2);
  if (err == ESP_OK) {
    *data = (read_buf[1] << 8) | read_buf[0]; // Little-endian format
  }
  return err;
}

esp_err_t BatteryManager::get_battery_voltage(float *voltage) {
  uint16_t raw_value;
  esp_err_t err = read_register(REG_VBAT_READ, &raw_value);
  if (err != ESP_OK) {
    return err;
  }

  // Apply mask to extract bits 12:1 (VBAT_ADC field)
  uint16_t vbat_adc = (raw_value & 0x1FFE) >> 1;
  // Convert to Volts using the 1.99mV per bit factor from the datasheet
  *voltage = (vbat_adc * 1.99) / 1000.0f;

  return ESP_OK;
}

esp_err_t BatteryManager::get_battery_temperature(float *percentage) {
  uint16_t raw_value;
  esp_err_t err = read_register(REG_TEMP_READ, &raw_value);
  if (err != ESP_OK) {
    return err;
  }

  // Apply mask to extract bits 11:0 (TS_ADC field)
  uint16_t temp_adc = (raw_value & 0xFFF);
  // Convert raw value to percentage of bias reference
  *percentage = temp_adc * 0.0961;
  return ESP_OK;
}

esp_err_t BatteryManager::get_charge_current(int16_t *current) {
  uint16_t raw_value;
  esp_err_t err = read_register(REG_CHARGE_CURRENT, &raw_value);
  if (err != ESP_OK) {
    return err;
  }

  // Check for the special error case (0x8000)
  if (raw_value == 0x8000) {
    ESP_LOGW(TAG, "Current polarity changed during measurement");
    *current = 0;
    return ESP_OK;
  }

  // Apply mask to extract bits 15:2 (TS_ADC field)
  int16_t charge_current = raw_value >> 2;

  // If the sign bit is set, convert to negative value
  if (charge_current & 0x2000) {
    // Extended the sign bit to 16 bits
    charge_current |= 0xC000;
    // Convert to positive and add - sign
    charge_current = -(~charge_current + 1);
  }

  // Convert to mA using the 4 mA per bit factor from the datasheet
  *current = charge_current * 4;
  return ESP_OK;
}