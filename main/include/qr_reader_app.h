#pragma once
#include "camera.h"
#include "freertos/FreeRTOS.h"
#include "qr_decoder.h"
#include "wifi.h"
#include <ArduinoJson.h>
#include <atomic>
#include <memory>

/**
 * @brief
 * QRReaderApp class is a singleton class that is responsible for running the QR
 * code reader application. It initializes and runs the QR code reader
 * application.
 *
 */
class QRReaderApp {
public:
  /**
   * @brief
   * Delete copy constructor and assignment operator
   *
   */
  QRReaderApp(const QRReaderApp &) = delete;
  QRReaderApp &operator=(const QRReaderApp &) = delete;

  /**
   * @brief
   * Returns the singleton instance of the QRReaderApp class.
   *
   * @return
   * QRReaderApp class instance.
   *
   */
  static QRReaderApp &getInstance() {
    static QRReaderApp instance;
    return instance;
  }

  /**
   * @brief
   * Start the QR code reader application.
   *
   */
  void start();
  /**
   * @brief
   * Stop the QR code reader application.
   *
   */
  void stop();

private:
  QRReaderApp();

  /**
   * @brief
   * Task function to run the QR code reader application.
   *
   * The task function runs the QR code reader application. The task runs until
   * the QR code is decoded or a shutdown is requested.
   *
   * @param arg
   * QRReaderApp object.
   *
   */
  static void qr_task(void *arg);

  /**
   * @brief Run the QR code reader application.
   *
   * The QR code reader uses the following steps:
   *
   *          1. Start the camera
   *
   *          2. Read the QR code
   *
   *          3. Connect to the WiFi network using the information from the QR
   *             code
   *          4. Send the GET request to acquire the static configuration from
   *             the server
   *
   *          5. Save the static configuration
   *
   *          6. Restart the device and start the camera app
   */
  void run();

  /**
   * @brief
   * Task function to decode the QR code.
   *
   * The task receives the camera frame buffer from the queue and decodes the QR
   * code using the QRDecoder class. The task runs until the QR code is decoded
   * or a shutdown is requested.
   *
   * @param arg
   * TaskContext object.
   *
   */
  static void qr_decode_task(void *arg);

  /**
   * @brief
   * Function to get the WiFi and server information from the QR code.
   *
   * The function starts the QR code decoding task, and starts taking pictures,
   * which will be sent for decoding. It runs until the QR code is decoded or a
   * shutdown is requested.
   *
   * The clean up of the QR code decoding task is done by this function.
   *
   */
  void get_qr_code();

  /**
   * @brief
   * Function to get the static configuration from the server.
   *
   * The function sends a GET request to the server to get the static
   * configuration. The function runs until the static configuration is received
   * or a shutdown is requested.
   *
   */
  void get_static_config();

  /**
   * @brief
   * Function to save the static configuration.
   *
   * The function saves the static configuration to the storage.
   *
   * @param doc
   * Static configuration in JSON format.
   *
   */
  void save_static_config(const JsonDocument &doc);

  /**
   * @brief Structure to pass the task context to the QR code decoder task
   * function
   *
   * The structure contains the QRDecoder object, the queue handle to receive
   * the camera frame buffer, and the flag to indicate if the QR code is
   * decoded.
   *
   * @param decoder QRDecoder object
   * @param queue Queue handle to receive the camera frame buffer
   * @param decoded Flag to indicate if the QR code is decoded
   *
   */
  struct TaskContext {
    QRDecoder decoder;
    QueueHandle_t queue;
    std::atomic<bool> &decoded;
    explicit TaskContext(int width, int height, QueueHandle_t q,
                         std::atomic<bool> &dec)
        : decoder(width, height), queue(q), decoded(dec) {}
  };

  Camera _cam;
  Wifi _wifi;

  static std::atomic<bool> _qr_code_decoded;
  TaskHandle_t _decode_task_handle = nullptr;
  TaskHandle_t _qr_app_task_handle = nullptr;
};