#include "charge_current.h"
#include "esp_log.h"

constexpr auto *TAG = "ChargeCurrent";

ChargeCurrent::ChargeCurrent(BatteryManager &battery_manager)
    : _battery_manager(battery_manager), _charge_current(0) {}

esp_err_t ChargeCurrent::init() { return _battery_manager.init(); }

esp_err_t ChargeCurrent::read() {
  esp_err_t err = _battery_manager.get_charge_current(&_charge_current);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to read charge current: %d", err);
    return err;
  }
  return ESP_OK;
}