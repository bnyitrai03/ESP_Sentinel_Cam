#pragma once

#include "esp_camera.h"
#include "quirc.h"
#include <memory>

class QRDecoder {
public:
  QRDecoder(int width, int height);

  // Delete copy constructor and assignment because of unique_ptr
  QRDecoder(const QRDecoder &) = delete;
  QRDecoder &operator=(const QRDecoder &) = delete;

  /*
   * @brief
   * Function to decode the QR code from the camera frame.
   *
   * The function extracts the camera frame buffer and decodes the QR code using
   * the quirc library. The function returns true if the QR code is decoded
   * successfully, otherwise false.
   *
   * @param frame
   * Camera frame buffer.
   *
   * @return
   *  - True if the QR code is decoded successfully
   *
   *  - False if the QR code is not decoded
   *
   */
  bool decode_frame(const camera_fb_t *frame);

private:
  /*
   * @brief
   * Function to save the decoded data to the storage.
   *
   * The function saves the decoded data to the NVS using the Storage class.
   *
   * @param data
   * Decoded data.
   *
   * @param length
   * Length of the decoded data.
   *
   */
  void save_decoded_data(const char *data, uint32_t length);

  struct QuircDeleter {
    void operator()(quirc *q) { quirc_destroy(q); }
  };
  std::unique_ptr<quirc, QuircDeleter> _qr;
};