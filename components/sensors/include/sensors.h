#pragma once

#include "isensor.h"
#include <ArduinoJson.h>
#include <esp_err.h>
#include <map>
#include <memory>

class Sensors {
public:
  Sensors();

  esp_err_t init();
  esp_err_t read_sensors(JsonDocument &doc);

private:
  std::map<std::string, std::unique_ptr<ISensor>> _sensors;
};