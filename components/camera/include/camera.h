#pragma once

#include "esp_camera.h"
#include "esp_log.h"
#include "pins.h"
#include <string>

/**
 * @brief Handles the OV5640 camera
 */
class Camera {
public:
  Camera() { ESP_LOGE("Camera", "This constructor is not supported"); };
  /*
   * @brief Constructor for the Camera class
   *
   * @param qr_reader_app: true if the QR reader app is running and false
   * otherwise
   *
   */
  Camera(bool qr_reader_app);

  /**
   * @brief Starts the camera
   *
   * @return
   *     - ESP_OK : camera initialized successfully
   *     - ESP_FAIL : couldn't initialize camera
   */
  esp_err_t start();

  /**
   * @brief Takes image
   *
   * @return
   *     - ESP_OK : captured image
   *
   *     - ESP_FAIL : couldn't take image
   */
  esp_err_t take_image();

  /**
   * @brief Gets the contents of the frame buffer of the captured image
   *
   * @return
   *     - Frame buffer of the captured image
   */
  const char *get_image_data() {
    return reinterpret_cast<const char *>(_fb->buf);
  }

  /**
   * @brief Gets image size
   *
   * @return
   *     - The size of the captured image
   */
  uint32_t get_image_size() { return _fb->len; }

  /*
   * @return The width of the captured image
   */
  int32_t get_width() { return _width; }

  /*
   * @return The height of the captured image
   */
  int32_t get_height() { return _height; }

  /*
   * @return The camera mode: GRAY or JPEG
   */
  const char *get_camera_mode() { return _camera_mode.c_str(); }

private:
  /**
   * @brief  Sets the framesize and pixformat based on the app mode and config
   *
   * @param size: The size of the image: FRAMESIZE_VGA or FRAMESIZE_WQXGA
   * @param format: The format of the image: JPEG or GRAY
   * @param qr_app: true if the QR reader app is running and false otherwise
   *
   */
  void set_camera_mode(framesize_t &size, pixformat_t &format, bool qr_app);

  /**
   * @brief Frame buffer containing the captured image
   * @private
   */
  camera_fb_t *_fb = nullptr;
  /**
   * @brief Configuration of the camera
   * @private
   */
  camera_config_t _config;

  std::string _camera_mode = "GRAY";
  int32_t _width = -1;
  int32_t _height = -1;
};