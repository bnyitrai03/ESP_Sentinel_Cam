#pragma once

#include "esp_camera.h"
#include "esp_log.h"
#include "pins.h"

/**
 * @brief Handles the OV5640 camera
 */
class Camera {
public:
  Camera() { ESP_LOGE("Camera", "This constructor is not supported"); };
  Camera(bool qr_reader_app);
  ~Camera() { esp_camera_fb_return(_fb); }

  /**
   * @brief Starts the camera
   *
   *
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

  /*
   * @brief Gets the frame buffer of the captured image
   *
   * @return
   *     - Frame buffer of the captured image
   */
  const camera_fb_t *get_fb() { return _fb; }

  /**
   * @brief Gets image size
   *
   * @return
   *     - The size of the captured image
   */
  uint32_t get_image_size() { return _fb->len; }

  /**
   * @brief Returns the frame buffer to enable the next image capture
   */
  void return_fb() { esp_camera_fb_return(_fb); }

private:
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
};