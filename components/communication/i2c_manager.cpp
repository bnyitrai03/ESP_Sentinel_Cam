#include "i2c_manager.h"
#include "esp_log.h"

constexpr auto *TAG = "I2C Manager";

I2CManager::I2CManager() : _initialized(false) {
  esp_log_level_set("sccb-ng", ESP_LOG_WARN);
}

I2CManager::~I2CManager() {
  if (_initialized) {
    i2c_del_master_bus(_bus_handle);
  }
}

esp_err_t I2CManager::init() {
  if (_initialized) {
    return ESP_OK;
  }

  i2c_master_bus_config_t i2c_bus_config = {
      .i2c_port = I2C_NUM_0,
      .sda_io_num = GPIO_NUM_4,
      .scl_io_num = GPIO_NUM_5,
      .clk_source = I2C_CLK_SRC_DEFAULT,
      .glitch_ignore_cnt = 7,
      .flags =
          {
              .enable_internal_pullup = 0,
          },
  };
  esp_err_t err = i2c_new_master_bus(&i2c_bus_config, &_bus_handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Creating I2C bus failed: %s", esp_err_to_name(err));
    return err;
  }

  _initialized = true;
  return err;
}

esp_err_t I2CManager::write(i2c_master_dev_handle_t device_handle,
                            const uint8_t *data, size_t length, int timeout) {
  if (!_initialized) {
    ESP_LOGE(TAG, "I2C Manager not initialized");
    return ESP_ERR_INVALID_STATE;
  }

  esp_err_t err = i2c_master_transmit(device_handle, data, length, timeout);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "I2C write failed: %s", esp_err_to_name(err));
  }
  return err;
}

esp_err_t I2CManager::read(i2c_master_dev_handle_t device_handle, uint8_t *data,
                           size_t length, int timeout) {
  if (!_initialized) {
    ESP_LOGE(TAG, "I2C Manager not initialized");
    return ESP_ERR_INVALID_STATE;
  }

  esp_err_t err = i2c_master_receive(device_handle, data, length, timeout);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "I2C read failed: %s", esp_err_to_name(err));
  }
  return err;
}

esp_err_t I2CManager::write_and_read(i2c_master_dev_handle_t device_handle,
                                     const uint8_t *write_data,
                                     size_t write_length, uint8_t *read_data,
                                     size_t read_length, int timeout) {
  if (!_initialized) {
    ESP_LOGE(TAG, "I2C Manager not initialized");
    return ESP_ERR_INVALID_STATE;
  }

  esp_err_t err = i2c_master_transmit_receive(
      device_handle, write_data, write_length, read_data, read_length, timeout);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "I2C write and read failed: %s", esp_err_to_name(err));
  }
  return err;
}

void I2CManager::probe(uint16_t address, int timeout) {
  if (!_initialized) {
    ESP_LOGE(TAG, "I2C manager not initialized");
    return;
  }

  esp_err_t err = i2c_master_probe(_bus_handle, address, timeout);

  switch (err) {
  case ESP_ERR_NOT_FOUND:
    ESP_LOGE(TAG, "The device 0x%02X is not connected!", address);
    break;

  case ESP_ERR_TIMEOUT:
    ESP_LOGE(TAG, "The I2C bus is busy or crashed");
    break;

  case ESP_OK:
    ESP_LOGI(TAG, "Device 0x%02X connected!", address);
    break;

  default:
    ESP_LOGE(TAG, "Unknown error!");
    break;
  }
}
