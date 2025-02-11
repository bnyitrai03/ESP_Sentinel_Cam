#include "esp_camera.h"

#define CAM_PIN_PWDN -1          /*!<GPIO_NUM_38 */
#define CAM_PIN_RESET -1         /*!<GPIO_NUM_39 */
#define CAM_PIN_XCLK GPIO_NUM_4  /*!<XC */
#define CAM_PIN_SIOD GPIO_NUM_15 /*!<SDA */
#define CAM_PIN_SIOC GPIO_NUM_16 /*!<SCL */

#define CAM_PIN_D7 GPIO_NUM_21   /*!<D9 */
#define CAM_PIN_D6 GPIO_NUM_47   /*!<D8 */
#define CAM_PIN_D5 GPIO_NUM_48   /*!<D7 */
#define CAM_PIN_D4 GPIO_NUM_45   /*!<D6 */
#define CAM_PIN_D3 GPIO_NUM_0    /*!<D5 */
#define CAM_PIN_D2 GPIO_NUM_17   /*!<D4 */
#define CAM_PIN_D1 GPIO_NUM_18   /*!<D3 */
#define CAM_PIN_D0 GPIO_NUM_8    /*!<D2 */
#define CAM_PIN_VSYNC GPIO_NUM_7 /*!<VS */
#define CAM_PIN_HREF GPIO_NUM_6  /*!<HS */
#define CAM_PIN_PCLK GPIO_NUM_5  /*!<PC */

/**
 * @brief Handles the camera
 */
class Camera {
public:
  Camera();
  ~Camera() { esp_camera_fb_return(_fb); }

  // there needs to be a delay before taking the first image
  // to guarantee sufficient exposition time

  /**
   * @brief Starts the camera
   *
   * A more indepth description could be added here if needed.
   *
   * @return
   *     - ESP_OK : camera initialized successfully
   *     - ESP_FAIL : couldn't initialize camera
   *     - ESP_ERR_NOT_SUPPORTED : JPEG format not supported on this sensor
   */
  esp_err_t start();

  /**
   * @brief Takes image
   *
   * @return
   *     - ESP_OK : captured image
   *     - ESP_FAIL : couldn't take image
   */
  esp_err_t take_image();

  /**
   * @brief Gets the contents of the frame buffer of the captured image
   *
   * @return
   *     - Frame buffer. In case of failure it returns a nullptr.
   */
  const char *get_image_data() {
    return reinterpret_cast<const char *>(_fb->buf);
  }

  /**
   * @brief Gets image size
   *
   * @return
   *     - The size of the frame buffer.
   */
  uint32_t get_image_size() { return _fb->len; }

  /**
   * @brief Returns the frame buffer
   */
  void return_fb() { esp_camera_fb_return(_fb); }

private:
  camera_fb_t *_fb = nullptr; /*!< Frame buffer */
  camera_config_t _config;    /*!< Config variable */
};