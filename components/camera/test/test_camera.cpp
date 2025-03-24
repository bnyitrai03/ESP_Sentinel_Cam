#include "camera.h"
#include "unity.h"

static Camera *test_camera = nullptr;

TEST_CASE("Get image data test", "[camera]") {
  test_camera = new Camera(false);
  test_camera->start();

  TEST_ASSERT_NOT_NULL(test_camera);
  TEST_ASSERT_EQUAL_STRING("GRAY",
                           test_camera->get_camera_mode()); // default mode
  TEST_ASSERT_EQUAL(ESP_OK, test_camera->take_image());
  TEST_ASSERT_NOT_NULL(test_camera->get_image_data());
  TEST_ASSERT_GREATER_THAN(0, test_camera->get_image_size());

  esp_camera_deinit();
  delete test_camera;
  test_camera = nullptr;
}

TEST_CASE("QR mode config test", "[camera]") {
  test_camera = new Camera(true);

  TEST_ASSERT_NOT_NULL(test_camera);
  TEST_ASSERT_EQUAL(640, test_camera->get_width());
  TEST_ASSERT_EQUAL(480, test_camera->get_height());

  esp_camera_deinit();
  delete test_camera;
  test_camera = nullptr;
}

TEST_CASE("Camera mode config test", "[camera]") {
  test_camera = new Camera(false);

  TEST_ASSERT_NOT_NULL(test_camera);
  TEST_ASSERT_EQUAL(2560, test_camera->get_width());
  TEST_ASSERT_EQUAL(1600, test_camera->get_height());

  esp_camera_deinit();
  delete test_camera;
  test_camera = nullptr;
}