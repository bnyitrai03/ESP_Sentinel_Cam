#include "camera.h"
#include "driver/rtc_io.h"
#include "error_handler.h"
#include "freertos/FreeRTOS.h"
#include "storage.h"

constexpr auto *TAG = "Camera";

Camera::Camera(bool qr_reader_app) {
  set_camera_deinit_callback([]() {
    esp_camera_deinit();
    gpio_set_level(CAM_PIN_PWDN, 1);
  });

  // raise the log level for the camera library
  esp_log_level_set("s3 ll_cam", ESP_LOG_WARN);
  esp_log_level_set("cam_hal", ESP_LOG_WARN);
  esp_log_level_set("sccb", ESP_LOG_WARN);
  esp_log_level_set("gpio", ESP_LOG_WARN);
  esp_log_level_set("camera", ESP_LOG_WARN);
  esp_log_level_set("ov5640", ESP_LOG_WARN);

  if (esp_reset_reason() == ESP_RST_DEEPSLEEP) {
    // reset camera pins for correct deep sleep wake up
    rtc_gpio_hold_dis(CAM_PIN_PWDN);
    rtc_gpio_hold_dis(CAM_PIN_XCLK);
    rtc_gpio_hold_dis(CAM_PIN_SIOD);
    rtc_gpio_hold_dis(CAM_PIN_SIOC);
    rtc_gpio_hold_dis(CAM_PIN_D7);
    rtc_gpio_hold_dis(CAM_PIN_D6);
    rtc_gpio_hold_dis(CAM_PIN_D5);
    rtc_gpio_hold_dis(CAM_PIN_D4);
    rtc_gpio_hold_dis(CAM_PIN_D3);
    rtc_gpio_hold_dis(CAM_PIN_D2);
    rtc_gpio_hold_dis(CAM_PIN_D1);
    rtc_gpio_hold_dis(CAM_PIN_D0);
    rtc_gpio_hold_dis(CAM_PIN_VSYNC);
    rtc_gpio_hold_dis(CAM_PIN_HREF);
    rtc_gpio_hold_dis(CAM_PIN_PCLK);
  }

  framesize_t size = FRAMESIZE_INVALID;
  pixformat_t format = PIXFORMAT_GRAYSCALE;
  set_camera_mode(size, format, qr_reader_app);

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

      .pixel_format = format,
      .frame_size = size,

      .jpeg_quality = 4, // 0-63
      .fb_count = 1, // When jpeg mode is used, if fb_count more than one, the
                     // driver will work in continuous mode.
      .fb_location = CAMERA_FB_IN_PSRAM,
      .grab_mode = CAMERA_GRAB_LATEST,
  };
}

void Camera::set_camera_mode(framesize_t &size, pixformat_t &format,
                             bool qr_app) {
  if (qr_app) {
    // when the qr reader app is running, use a lower resolution due to faster
    // decoding
    size = FRAMESIZE_VGA;
    _width = 640;
    _height = 480;
  } else {
    size = FRAMESIZE_WQXGA;
    _width = 2560;
    _height = 1600;
    // set the pixel format in the camera app mode based on the static config
    char camera_mode[6] = {0};
    if (Storage::read("cameraMode", camera_mode, sizeof(camera_mode)) !=
        ESP_OK) {
      ESP_LOGE(TAG, "Failed to read camera mode from NVS");
      ESP_LOGW(TAG, "Camera mode set to default: GRAY");
    }
    std::string camera_mode_str(camera_mode);

    if (camera_mode_str == "COLOR") {
      format = PIXFORMAT_JPEG;
      _camera_mode = "COLOR";
    }
  }
}

esp_err_t Camera::start() {
  esp_err_t err = esp_camera_init(&_config);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Camera Init Failed");
    restart();
  }
  gpio_set_level(CAM_PIN_PWDN, 0); // Power on the camera
  return err;
}

esp_err_t Camera::take_image() {
  // flush the old frame buffer
  _fb = esp_camera_fb_get();
  if (!_fb) {
    ESP_LOGE(TAG, "Failed to capture image");
    return ESP_FAIL;
  }
  esp_camera_fb_return(_fb);

  // capture new image
  _fb = esp_camera_fb_get();
  if (!_fb) {
    ESP_LOGE(TAG, "Failed to capture image");
    return ESP_FAIL;
  }
  ESP_LOGI(TAG, "Image taken!");
  return ESP_OK;
}