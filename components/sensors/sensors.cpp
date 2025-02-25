#include "sensors.h"

esp_err_t Sensors::read_sensors(JsonDocument &doc) {
  doc["cpuTemp"] = 25.0;
  doc["batteryCharge"] = 100;
  doc["luminosity"] = 1500;
  doc["chargeCurrent"] = 450;
  return ESP_OK;
}
