#include "storage.h"
#include "error_handler.h"
#include "esp_log.h"
#include "nvs.h"
#include "nvs_handle.hpp"

constexpr auto *TAG = "storage";

Storage::Storage() {
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    nvs_flash_erase();
    if (nvs_flash_init() != ESP_OK) {
      ESP_LOGE(TAG, "Failed to init flash!");
      restart();
    }
  }
  if (ret != ESP_OK && ret != ESP_ERR_NVS_NEW_VERSION_FOUND &&
      ret != ESP_ERR_NVS_NO_FREE_PAGES) {
    ESP_LOGE(TAG, "Failed to init flash!");
    restart();
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

esp_err_t Storage::read(const std::string &key, char *value, uint32_t len) {
  esp_err_t err;
  // Handle will automatically close when going out of scope
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

  err = handle->commit();
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Error comitting to NVS: (%s)", esp_err_to_name(err));
  }
  return err;
}