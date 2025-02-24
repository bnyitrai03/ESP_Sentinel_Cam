#include "camera.h"
#include "error_handler.h"
#include "freertos/FreeRTOS.h"
#include <esp_log.h>

constexpr auto *TAG = "Camera";

Camera::Camera() {
  _config = {
      .pin_pwdn = CAM_PIN_PWDN,
      .pin_reset = CAM_PIN_RESET,
      .pin_xclk = CAM_PIN_XCLK,
      .pin_sccb_sda = CAM_PIN_SIOD,
      .pin_sccb_scl = CAM_PIN_SIOC,

      .pin_d7 = CAM_PIN_D7,
      .pin_d6 = CAM_PIN_D6,
      .pin_d5 = CAM_PIN_D5,
      .pin_d4 = CAM_PIN_D4,
      .pin_d3 = CAM_PIN_D3,
      .pin_d2 = CAM_PIN_D2,
      .pin_d1 = CAM_PIN_D1,
      .pin_d0 = CAM_PIN_D0,
      .pin_vsync = CAM_PIN_VSYNC,
      .pin_href = CAM_PIN_HREF,
      .pin_pclk = CAM_PIN_PCLK,

      .xclk_freq_hz = 24000000, // XCLK 24MHz for OV5640
      .ledc_timer = LEDC_TIMER_0,
      .ledc_channel = LEDC_CHANNEL_0,

      .pixel_format = PIXFORMAT_GRAYSCALE, // PIXFORMAT_GRAYSCALE
      .frame_size = FRAMESIZE_WQXGA,

      .jpeg_quality = 4, // 0-63
      .fb_count = 1, // When jpeg mode is used, if fb_count more than one, the
                     // driver will work in continuous mode.
      .fb_location = CAMERA_FB_IN_PSRAM,
      .grab_mode = CAMERA_GRAB_LATEST,
  };
  set_camera_deinit_callback([]() { esp_camera_deinit(); });
}

esp_err_t Camera::start() {
  esp_err_t err = esp_camera_init(&_config);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Camera Init Failed");
    restart();
  }
  return err;
}

esp_err_t Camera::take_image() {
  // flush the old frame buffer
  _fb = esp_camera_fb_get();
  if (!_fb) {
    ESP_LOGE(TAG, "Failed to flush old image");
    return ESP_FAIL;
  }
  return_fb();

  // capture new image
  _fb = esp_camera_fb_get();
  if (!_fb) {
    ESP_LOGE(TAG, "Failed to capture image");
    return ESP_FAIL;
  }
  ESP_LOGI(TAG, "Image taken!");
  return ESP_OK;
}