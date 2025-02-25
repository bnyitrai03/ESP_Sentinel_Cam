#pragma once

#include <ArduinoJson.h>
#include <esp_err.h>

// reading the different sensors can be implemnted in paralell tasks
class Sensors {
public:
  esp_err_t read_sensors(JsonDocument &doc);
};