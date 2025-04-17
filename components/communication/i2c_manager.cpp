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
  // TODO: remove this
  /*if (_initialized) {
    return ESP_OK;
  }*/

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

esp_err_t I2CManager::probe(uint16_t address, int timeout) {
  if (!_initialized) {
    ESP_LOGE(TAG, "I2C manager not initialized");
    return ESP_FAIL;
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

  return err;
}

esp_err_t I2CManager::reset() {
  if (!_initialized) {
    ESP_LOGE(TAG, "I2C manager not initialized");
    return ESP_FAIL;
  }

  esp_err_t err = i2c_del_master_bus(_bus_handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Resetting I2C bus failed: %s", esp_err_to_name(err));
    return err;
  }

  err = this->init();
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Reinitializing I2C bus failed: %s", esp_err_to_name(err));
  }

  return err;
}