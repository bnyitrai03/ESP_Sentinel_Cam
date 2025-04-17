#include "battery_manager.h"
#include "esp_log.h"

constexpr auto *TAG = "BatteryManager";

BatteryManager::BatteryManager(I2CManager &i2c)
    : _initialized(false), _i2c(i2c) {}

BatteryManager::~BatteryManager() {
  i2c_master_bus_rm_device(_device_handle);

  if (_initialized) {
    this->disable_ADC();
  }
}

esp_err_t BatteryManager::init() {
  if (_initialized) {
    return ESP_OK;
  }

  i2c_master_bus_handle_t bus_handle = _i2c.get_bus_handle();
  i2c_device_config_t dev_cfg = {
      .dev_addr_length = I2C_ADDR_BIT_LEN_7,
      .device_address = I2C_DEVICE_ADDRESS_NOT_USED,
      .scl_speed_hz = 100000,
  };

  esp_err_t err =
      i2c_master_bus_add_device(bus_handle, &dev_cfg, &_device_handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to add device to I2C bus: %d", err);
    return err;
  }

  // Probe device to check if it's connected
  if (_i2c.probe(BQ25622_ADDR) != ESP_OK) {
    return ESP_ERR_NOT_FOUND;
  }

  // Enable ADC
  _initialized = true;
  if (measure_adc_enabled) {
    this->enable_ADC();
  }
  return ESP_OK;
}

esp_err_t BatteryManager::write_register(uint8_t reg, uint8_t data) {
  if (!_initialized) {
    return ESP_ERR_INVALID_STATE;
  }

  // Prepare the I2C data to write
  uint8_t address = (BQ25622_ADDR << 1); // LSB = 0 for write
  uint8_t write_data[3] = {address, reg, data};

  // BQ25622 single register write operation sequence
  i2c_operation_job_t write[] = {
      {.command = I2C_MASTER_CMD_START},
      {.command = I2C_MASTER_CMD_WRITE,
       .write = {.ack_check = true, .data = write_data, .total_bytes = 3}},
      {.command = I2C_MASTER_CMD_STOP},
  };

  esp_err_t err =
      i2c_master_execute_defined_operations(_device_handle, write, 3, 1000);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to write register 0x%02X: %s", reg,
             esp_err_to_name(err));
  }
  return err;
}

esp_err_t BatteryManager::read_register(uint8_t reg, uint16_t *data) {
  if (!_initialized) {
    return ESP_ERR_INVALID_STATE;
  }

  uint8_t read_address = (BQ25622_ADDR << 1 | 1); // LSB = 1 for read
  uint8_t write_address = (BQ25622_ADDR << 1);    // LSB = 0 for write

  uint8_t write_data[2] = {write_address, reg};
  uint8_t read_data[2] = {};
  *data = 0;

  i2c_operation_job_t read[] = {
      // write register address
      {.command = I2C_MASTER_CMD_START},
      {.command = I2C_MASTER_CMD_WRITE,
       .write = {.ack_check = true, .data = write_data, .total_bytes = 2}},

      // write read command
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
      {.command = I2C_MASTER_CMD_STOP},
  };

  esp_err_t err =
      i2c_master_execute_defined_operations(_device_handle, read, 7, 1000);

  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to read register 0x%02X: %s", reg,
             esp_err_to_name(err));
    return err;
  }

  // Combine the two bytes into a single 16-bit value
  *data = (read_data[1] << 8) | read_data[0]; // little endian
  return err;
}

void BatteryManager::enable_ADC() {
  uint16_t adc_control = 0;
  esp_err_t err = read_register(REG_ADC_CONTROL, &adc_control);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to read the ADC register: %s", esp_err_to_name(err));
    return;
  }

  adc_control =
      (adc_control | ADC_ENABLE | ADC_AVG | ADC_AVG_INIT) & ADC_SAMPLE;
  err = write_register(REG_ADC_CONTROL, adc_control);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to write the ADC control register: %s",
             esp_err_to_name(err));
  }
}

void BatteryManager::disable_ADC() {
  if (!_initialized) {
    return;
  }

  uint16_t adc_control = 0;
  esp_err_t err = read_register(REG_ADC_CONTROL, &adc_control);
  if (err == ESP_OK) {
    adc_control &= ~ADC_ENABLE;
    err = write_register(REG_ADC_CONTROL, adc_control);
  }

  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to disable ADC: %s", esp_err_to_name(err));
  }
}

// ---------------------------- Sensor Functions -----------------------

esp_err_t BatteryManager::get_battery_voltage(float *voltage) {
  *voltage = 0.0f;
  if (!_initialized) {
    return ESP_ERR_INVALID_STATE;
  }

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
  *percentage = 0.0f;
  if (!_initialized) {
    return ESP_ERR_INVALID_STATE;
  }

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
  *current = 0;
  if (!_initialized) {
    return ESP_ERR_INVALID_STATE;
  }

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