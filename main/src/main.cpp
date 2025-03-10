#include "button.h"
#include "camera_app.h"
#include "esp_log.h"
#include "led.h"
#include "qr_reader_app.h"
#include "secret.h"
#include "storage.h"
#include <ArduinoJson.h>
#include <esp_log.h>

constexpr auto *TAG = "main_app";

extern "C" void app_main(void) {
  Storage storage;
  Led led;
  Button button;

  char app_mode[4] = {0};
  Storage::read("app", app_mode, sizeof(app_mode));
  if (strcmp(app_mode, "cam") == 0) {
    ESP_LOGI(TAG, "Starting the starling detection mode");
    Led::set_pattern(Led::Pattern::ON);
    CameraApp &app = CameraApp::getInstance(button);
    app.start();
    button.register_shutdown_observer(xTaskGetCurrentTaskHandle());
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    app.stop();
    led.stop();
    while (true) {
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  } else {
    ESP_LOGI(TAG, "Starting the QR code reader mode");
    Led::set_pattern(Led::Pattern::NO_QR_CODE_BLINK);
    QRReaderApp &app = QRReaderApp::getInstance();
    app.run();
  }

  // ------------------------------------------------------------------
}