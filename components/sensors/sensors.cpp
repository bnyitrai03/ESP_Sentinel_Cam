#include "sensors.h"
#include "battery_charge.h"
#include "cpu_temp.h"
#include "light_sensor.h"
#include "myadc.h"
#include <esp_log.h>

constexpr auto *TAG = "Sensors";

Sensors::Sensors() {
  _sensors["batteryCharge"] = std::make_unique<BatteryCharge>();
  _sensors["cpuTemp"] = std::make_unique<CpuTemp>();
  _sensors["luminosity"] = std::make_unique<LightSensor>();
}

esp_err_t Sensors::init() {
  for (auto &[name, sensor] : _sensors) {
    esp_err_t err = sensor->init();
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Failed to initialize sensor: %s", name.c_str());
      return err;
    }
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
  // TODO: Implement these sensors
  doc["batteryTemp"] = 0.0f;
  doc["chargeCurrent"] = 0;

  return ESP_OK;
}
