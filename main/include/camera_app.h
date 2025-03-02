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
 * camera application. It initializes and
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

  /**
   * @brief
   * Runs the image capturing application.
   *
   */
  void run();

  static void request_shutdown() { shutdown_requested = true; }

private:
  CameraApp();

  esp_err_t send_json(JsonDocument &doc, const char *topic);
  esp_err_t send_health_report();
  esp_err_t send_image_header(const char *timestamp);
  void send_image();

  /*
   * @brief
   * Calculates the maximum time to wait for receiving an acknowledgement.
   *
   * @return
   * The maximum wait time in milliseconds.
   *
   */
  uint32_t calculate_max_wait();

  static std::atomic<bool> shutdown_requested;
  Camera _cam;
  Wifi _wifi;
  MQTT _mqtt;
  Config _config;
  Sensors _sensors;
};