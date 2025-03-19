#pragma once

#include "button.h"
#include "camera.h"
#include "camera_app.h"
#include "config.h"
#include "error_handler.h"
#include "event_manager.h"
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
  /**
   * @brief
   * Delete copy constructor and assignment operator
   *
   */
  CameraApp(const CameraApp &) = delete;
  CameraApp &operator=(const CameraApp &) = delete;

  /**
   * @brief
   * Returns the singleton instance of the CameraApp class.
   *
   * @return
   * CameraApp class instance.
   *
   */
  static CameraApp &getInstance() {
    static CameraApp instance;
    return instance;
  }

  /**
   * @brief
   * Creates the camera task and starts the camera application.
   *
   */
  void start();
  /**
   * @brief
   * Stops the camera task and the camera application.
   *
   */
  void stop();

private:
  CameraApp();

  /**
   * @brief
   * Runs the image capturing application.
   *
   */
  void run();
  /**
   *
   * @brief
   * Starts the camera task, and signals when it finishes.
   *
   * @param pvParameters
   * CameraApp instance
   *
   */
  static void camera_task(void *pvParameters);

  /**
   * @brief Initializes all components needed for the camera application
   * @return true if initialization was successful, false otherwise
   */
  bool initialize();
  /**
   *
   * @brief Handles configuration updates and health reporting
   * @return true if configuration was handled successfully, false otherwise
   */
  bool handle_config_update();
  /**
   * @brief Captures an image and sends it to the MQTT broker
   * @return true if image was captured and sent successfully, false otherwise
   */
  bool capture_and_send_image();

  /*
   * @brief
   * Publishes the JSON document to the given MQTT topic.
   *
   */
  esp_err_t send_json(JsonDocument &doc, const char *topic);
  /**
   * @brief
   * Assemble and send the health report to the MQTT broker.
   *
   * The health report contains the timestamp, configuration ID, period, and
   * sensor readings.
   *
   */
  esp_err_t send_health_report();
  /**
   * @brief
   * Assemble and send the image header to the MQTT broker.
   *
   * The image header contains the timestamp, the image size and the camera
   * color mode.
   *
   */
  esp_err_t send_image_header(const char *timestamp);
  /**
   * @brief
   * Send the image to the MQTT broker.
   *
   */
  esp_err_t send_image();
  /*
   * @brief
   * Calculates the maximum time to wait for receiving an acknowledgement.
   *
   * @return
   * The maximum wait time in milliseconds.
   *
   */
  uint32_t calculate_max_wait();

  std::atomic<bool> _stopped_from_other_task{false};
  TaskHandle_t _camera_task_handle = nullptr;
  Camera _cam;
  Wifi _wifi;
  MQTT _mqtt;
  Config _config;
  Sensors _sensors;
};