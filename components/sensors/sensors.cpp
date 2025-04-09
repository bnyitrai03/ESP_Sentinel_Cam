#include "sensors.h"
#include "battery_charge.h"
#include "battery_temp.h"
#include "charge_current.h"
#include "cpu_temp.h"
#include "light_sensor.h"
#include <esp_log.h>
#include <vector>

constexpr auto *TAG = "Sensors";

Sensors::Sensors()
    : _i2c_manager(I2CManager()), _battery_manager(_i2c_manager) {

  _sensors["batteryCharge"] = std::make_unique<BatteryCharge>(_battery_manager);
  _sensors["chargeCurrent"] = std::make_unique<ChargeCurrent>(_battery_manager);
  _sensors["batteryTemp"] = std::make_unique<BatteryTemp>(_battery_manager);
  _sensors["cpuTemp"] = std::make_unique<CpuTemp>();
  //_sensors["luminosity"] = std::make_unique<LightSensor>(_i2c_manager);
}

esp_err_t Sensors::init() {
  if (_i2c_manager.init() != ESP_OK) {
    ESP_LOGE(TAG, "Failed to initialize I2C bus!");
    return ESP_FAIL;
  }

  if (_battery_manager.init() != ESP_OK) {
    ESP_LOGE(TAG, "Failed to initialize BatteryManager!");
    return ESP_FAIL;
  }

  std::vector<std::string> sensors_to_remove;
  for (auto &[name, sensor] : _sensors) {
    esp_err_t err = sensor->init();
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Failed to initialize sensor: %s", name.c_str());
      sensors_to_remove.push_back(name);
    }
  }

  for (const auto &name : sensors_to_remove) {
    _sensors.erase(name);
    ESP_LOGW(TAG, "Removing sensor: %s", name.c_str());
  }

  return ESP_OK;
}

esp_err_t Sensors::read_sensors(JsonDocument &doc) {
  for (auto &[name, sensor] : _sensors) {
    esp_err_t err = sensor->read();
    if (err == ESP_OK) {
      doc[name] = sensor->get_value();
    } else {
      ESP_LOGE(TAG, "Failed to read sensor: %s", name.c_str());
      doc[name] = 0;
    }
  }

  _battery_manager.disable_ADC();
  return ESP_OK;
}