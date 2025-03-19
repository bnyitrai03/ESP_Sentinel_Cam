#include "button.h"
#include "camera_app.h"
#include "esp_log.h"
#include "event_manager.h"
#include "led.h"
#include "mysleep.h"
#include "qr_reader_app.h"
#include "secret.h"
#include "storage.h"
#include <ArduinoJson.h>
#include <esp_log.h>

constexpr auto *TAG = "Main app";

void main_task(void *pvParameter);
void run_camera_app_mode();
void run_qr_reader_mode();
void setup_camera_mode_events(CameraApp &app, Button &button, Led &led);
void setup_qr_reader_mode_events(QRReaderApp &app, Button &button, Led &led);

// *********************** MAIN APP ***********************
extern "C" void app_main(void) {
  auto res = xTaskCreate(&main_task, "main_task", 4096, NULL, 15, NULL);
  if (res != pdPASS) {
    ESP_LOGE(TAG, "Failed to create main task");
    restart();
  }
}

void main_task(void *pvParameter) {
  Storage storage;

  char app_mode[4] = {0};
  Storage::read("mode", app_mode, sizeof(app_mode));
  if (strcmp(app_mode, "cam") == 0) {
    run_camera_app_mode();
  } else {
    run_qr_reader_mode();
  }

  vTaskDelete(NULL);
}
// *********************************************************

/*
 * Camera application mode
 * 1. Start the camera app
 * 2. Subscribe to the events
 * 3. Start the event manager, which handles the inter task communication
 * 4. Wait for events
 * 5. When an event occurs, process the event queue and handle the event
 */
void run_camera_app_mode() {
  EventManager &event_manager = EventManager::getInstance();
  Led led;
  Button button;
  ESP_LOGI(TAG, "Starting the starling detection mode");
  Led::set_pattern(Led::Pattern::ON);

  CameraApp &app = CameraApp::getInstance();
  setup_camera_mode_events(app, button, led);
  app.start();

  while (true) {
    event_manager.process_event_queue();
  }
}

/*
 * QR code reader application mode works similarly to the camera app mode.
 *
 */
void run_qr_reader_mode() {
  EventManager &event_manager = EventManager::getInstance();
  Led led;
  Button button;

  ESP_LOGI(TAG, "Starting the QR code reader mode");
  Led::set_pattern(Led::Pattern::NO_QR_CODE_BLINK);
  QRReaderApp &app = QRReaderApp::getInstance();
  setup_qr_reader_mode_events(app, button, led);
  app.start();

  while (true) {
    event_manager.process_event_queue();
  }
}

/*
 * Setup the camera mode events:
 *    - BUTTON_PRESSED: Stop the camera app, wait for the button release
 *    - SLEEP_UNTIL_NEXT_PERIOD: The camera app finished running, and based on
 *                               the period, the device will sleep until the
 *                               next image transfer
 *    - SLEEP_UNTIL_NEXT_TIMING: Based on the timing configuration, the device
 *                               will sleep until the start of the next timing
 *    - SLEEP_UNTIL_BUTTON_PRESS: The button was pressed for a short duration,
 *                                the device will sleep until the next button
 *                                press
 *    - RESET: The button was long pressed (> 2sec), the device will reset
 *
 */
void setup_camera_mode_events(CameraApp &app, Button &button, Led &led) {
  SUBSCRIBE(EventType::BUTTON_PRESSED, { app.stop(); });

  SUBSCRIBE(EventType::SLEEP_UNTIL_NEXT_PERIOD, {
    button.stop();
    led.stop();
    ESP_LOGW(TAG, "Device going to sleep until next period!");
    deinit_components();
    mysleep(static_cast<uint64_t>(Config::get_period()));
  });

  SUBSCRIBE(EventType::SLEEP_UNTIL_NEXT_TIMING, {
    button.stop();
    led.stop();
    deinit_components();
    TimingConfig timing = Config::get_active_config();
    mysleep(timing.end);
  });

  SUBSCRIBE(EventType::SLEEP_UNTIL_BUTTON_PRESS, {
    led.stop();
    ESP_LOGW(TAG, "Device going to sleep until button press!");
    deinit_components();
    button_press_sleep();
  });

  SUBSCRIBE(EventType::RESET, {
    led.stop();
    reset_device();
  });
}

/*
 * Setup the QR reader mode events:
 *    - BUTTON_PRESSED: Stop the QR reader app, wait for the button release
 *    - SLEEP_UNTIL_BUTTON_PRESS: The button was pressed for a short duration,
 *                                the device will sleep until the next button
 *                                press
 *    - RESET: The button was long pressed (> 2sec), the device will reset
 *    - STOP_BUTTON: Stop the button task
 *
 */
void setup_qr_reader_mode_events(QRReaderApp &app, Button &button, Led &led) {
  SUBSCRIBE(EventType::BUTTON_PRESSED, { app.stop(); });

  SUBSCRIBE(EventType::STOP_BUTTON, { button.stop(); });

  SUBSCRIBE(EventType::SLEEP_UNTIL_BUTTON_PRESS, {
    led.stop();
    ESP_LOGW(TAG, "Device going to sleep until button press!");
    deinit_components();
    button_press_sleep();
  });

  SUBSCRIBE(EventType::RESET, {
    led.stop();
    reset_device();
  });
}