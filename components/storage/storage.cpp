#include "storage.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs.h"
#include "nvs_handle.hpp"
#include <nvs_flash.h>

constexpr auto *TAG = "Storage";

Storage::Storage() {
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    nvs_flash_erase();
    if (nvs_flash_init() != ESP_OK) {
      ESP_LOGE(TAG, "Failed to init flash!");
      esp_restart();
    }
  }
  if (ret != ESP_OK && ret != ESP_ERR_NVS_NEW_VERSION_FOUND &&
      ret != ESP_ERR_NVS_NO_FREE_PAGES) {
    ESP_LOGE(TAG, "Failed to init flash!");
    esp_restart();
  }
}

esp_err_t Storage::write(const std::string &key, const std::string &value) {
  esp_err_t err;
  // Handle will automatically close when going out of scope
  std::unique_ptr<nvs::NVSHandle> handle =
      nvs::open_nvs_handle("storage", NVS_READWRITE, &err);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    return err;
  }

  err = handle->set_string(key.c_str(), value.c_str());
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Error writing to NVS: (%s)", esp_err_to_name(err));
    return err;
  }

  err = handle->commit();
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Error comitting to NVS: (%s)", esp_err_to_name(err));
  }
  return err;
}

esp_err_t Storage::write(const std::string &key, uint32_t value) {
  esp_err_t err;
  std::unique_ptr<nvs::NVSHandle> handle =
      nvs::open_nvs_handle("storage", NVS_READWRITE, &err);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    return err;
  }

  err = handle->set_item(key.c_str(), value);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Error writing to NVS: (%s)", esp_err_to_name(err));
    return err;
  }

  err = handle->commit();
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Error comitting to NVS: (%s)", esp_err_to_name(err));
  }
  return err;
}

esp_err_t Storage::read(const std::string &key, char *value, uint32_t len) {
  esp_err_t err;
  std::unique_ptr<nvs::NVSHandle> handle =
      nvs::open_nvs_handle("storage", NVS_READONLY, &err);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    return err;
  }

  err = handle->get_string(key.c_str(), value, len);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Error reading from NVS: (%s)", esp_err_to_name(err));
    return err;
  }

  return err;
}

esp_err_t Storage::read_error_count(uint32_t *value) {
  esp_err_t err;
  std::unique_ptr<nvs::NVSHandle> handle =
      nvs::open_nvs_handle("storage", NVS_READONLY, &err);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    return err;
  }

  err = handle->get_item("error_count", *value);
  switch (err) {
  case ESP_OK:
    break;
  case ESP_ERR_NVS_NOT_FOUND:
    *value = 1;
    ESP_LOGW(TAG, "Error count not found, initializing to 1");
    break;
  default:
    ESP_LOGE(TAG, "Error (%s) reading!\n", esp_err_to_name(err));
  }

  return err;
}