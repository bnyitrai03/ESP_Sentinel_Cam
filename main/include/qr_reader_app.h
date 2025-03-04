#pragma once
#include "camera.h"
#include "qr_decoder.h"
#include "wifi.h"
#include <atomic>
#include <memory>

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

  void run();
  static void request_shutdown() { _shutdown_requested = true; }

private:
  QRReaderApp();

  /*
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

  /*
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

  /*
   * @brief Structure to pass the task context to the QR code decoder task
   * function
   *
   * The structure contains the QRDecoder object, the queue handle to receive
   * the camera frame buffer, and the flag to indicate if the QR code is
   * decoded.
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
  static std::atomic<bool> _shutdown_requested;
  static std::atomic<bool> _qr_code_decoded;
};