#include "esp_camera.h"
#include "pins.h"
//comment
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
  size_t get_image_size() { return _fb->len; }

  /**
  * @brief Returns the frame buffer
  */
  void return_fb() { esp_camera_fb_return(_fb); }

private:
  camera_config_t _config; /*!< Config variable */
  camera_fb_t *_fb = nullptr; /*!< Frame buffer */
};