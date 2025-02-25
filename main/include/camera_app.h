#pragma once

#include "camera.h"
#include "camera_app.h"
#include "config.h"
#include "error_handler.h"
#include "mqtt.h"
#include "mytime.h"
#include "sensors.h"
#include "storage.h"
#include "wifi.h"
#include <ArduinoJson.h>
#include <atomic>

/*
 * @brief
 * CameraApp class is a singleton class that is responsible for running the
 * camera application. It initializes the camera, wifi, and mqtt objects and
 * runs the camera application.
 *
 */
class CameraApp {
public:
  CameraApp(const CameraApp &) = delete;
  CameraApp &operator=(const CameraApp &) = delete;

  static CameraApp &getInstance() {
    static CameraApp instance;
    return instance;
  }

  void run();

  static void request_shutdown() { shutdown_requested = true; }

private:
  CameraApp();

  esp_err_t send_json(JsonDocument &doc, const char *topic);
  esp_err_t send_health_report();

  static std::atomic<bool> shutdown_requested;
  Camera _cam;
  Wifi _wifi;
  MQTT _mqtt;
  Config _config;
  Sensors _sensors;
};