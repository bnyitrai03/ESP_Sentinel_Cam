#include "rgb_led.h"
#include "driver/rtc_io.h"
#include "esp_log.h"
#include "esp_system.h"

static const char *TAG = "RGB_LED";

// PWM configuration
constexpr ledc_timer_t LEDC_TIMER = LEDC_TIMER_0;
constexpr ledc_mode_t LEDC_MODE = LEDC_LOW_SPEED_MODE;
constexpr ledc_channel_t RED_CHANNEL = LEDC_CHANNEL_0;
constexpr ledc_channel_t GREEN_CHANNEL = LEDC_CHANNEL_1;
constexpr ledc_channel_t BLUE_CHANNEL = LEDC_CHANNEL_2;
constexpr uint32_t LEDC_FREQUENCY = 5000; // 5kHz PWM frequency

RGBLed::Pattern RGBLed::_current_pattern = RGBLed::Pattern::OFF;
bool RGBLed::_led_state = false;
SemaphoreHandle_t RGBLed::_pattern_mutex = xSemaphoreCreateMutex();
SemaphoreHandle_t RGBLed::_led_mutex = xSemaphoreCreateMutex();

// Define pattern colors
const RGBLed::RGBColor RGBLed::PATTERN_COLORS[] = {
    {0, 0, 0},       // OFF - No color
    {255, 255, 255}, // ON - White
    {0, 0, 255},     // NO_QR_CODE - Blue
    {0, 255, 0},     // STATIC_CONFIG_SAVED - Green
    {255, 165, 0},   // MQTT_CONNECTED - Orange
    {255, 0, 0}      // ERROR_BLINK - Red
};

RGBLed::RGBLed() {
  if (!_pattern_mutex || !_led_mutex) {
    ESP_LOGE(TAG, "Failed to create mutexes");
    esp_restart();
  }

  // Initialize the ledc
  if (!init_hardware()) {
    ESP_LOGE(TAG, "Failed to initialize RGB LED hardware");
    esp_restart();
  }

  _running = true;
  BaseType_t res =
      xTaskCreate(rgb_led_task, "rgb_led_task", 3000, this, 2, nullptr);
  if (res != pdPASS) {
    ESP_LOGE(TAG, "Failed to create RGB LED task");
    esp_restart();
  }
}

RGBLed::~RGBLed() {
  // Signal the task to stop
  stop();
}

void RGBLed::stop() {
  _running = false;
  ESP_LOGI(TAG, "RGB LED task stopped");
}

void RGBLed::set_pattern(Pattern pattern) {
  if (!_pattern_mutex) {
    ESP_LOGE(TAG, "Pattern mutex not initialized");
    return;
  }

  if (xSemaphoreTake(_pattern_mutex, pdMS_TO_TICKS(500)) == pdTRUE) {
    _current_pattern = pattern;
    xSemaphoreGive(_pattern_mutex);
  } else {
    ESP_LOGW(TAG, "Failed to acquire pattern mutex");
  }
}

void RGBLed::set_rgb(uint8_t r, uint8_t g, uint8_t b) {
  if (!_led_mutex) {
    ESP_LOGE(TAG, "LED mutex not initialized");
    return;
  }

  if (xSemaphoreTake(_led_mutex, pdMS_TO_TICKS(500)) == pdTRUE) {
    ledc_set_duty(LEDC_MODE, RED_CHANNEL, r);
    ledc_set_duty(LEDC_MODE, GREEN_CHANNEL, g);
    ledc_set_duty(LEDC_MODE, BLUE_CHANNEL, b);

    ledc_update_duty(LEDC_MODE, RED_CHANNEL);
    ledc_update_duty(LEDC_MODE, GREEN_CHANNEL);
    ledc_update_duty(LEDC_MODE, BLUE_CHANNEL);

    xSemaphoreGive(_led_mutex);
  } else {
    ESP_LOGW(TAG, "Failed to acquire LED mutex");
  }
}

void RGBLed::set_color(Pattern pattern) {
  // Get pattern index to access the right color
  int pattern_index = static_cast<int>(pattern);

  // Set the color for this pattern
  const RGBColor &color = PATTERN_COLORS[pattern_index];
  set_rgb(color.r, color.g, color.b);
}

void RGBLed::blink_led(Pattern pattern) {
  // Toggle LED state - true means LED is ON
  if (_led_state) {
    // LED is currently ON, turn it OFF
    set_rgb(0, 0, 0);
    _led_state = false;
  } else {
    // LED is currently OFF, turn it ON with pattern color
    set_color(pattern);
    _led_state = true;
  }
}

bool RGBLed::init_hardware() {
  if (esp_reset_reason() == ESP_RST_DEEPSLEEP) {
    gpio_hold_dis(RED_PIN);
    gpio_hold_dis(GREEN_PIN);
    gpio_hold_dis(BLUE_PIN);
  }

  ledc_timer_config_t ledc_timer = {.speed_mode = LEDC_MODE,
                                    .duty_resolution = LEDC_TIMER_8_BIT,
                                    .timer_num = LEDC_TIMER,
                                    .freq_hz = LEDC_FREQUENCY,
                                    .clk_cfg = LEDC_AUTO_CLK};

  if (ledc_timer_config(&ledc_timer) != ESP_OK) {
    ESP_LOGE(TAG, "Failed to configure LEDC timer");
    return false;
  }

  ledc_channel_config_t red_channel = {.gpio_num = RED_PIN,
                                       .speed_mode = LEDC_MODE,
                                       .channel = RED_CHANNEL,
                                       .intr_type = LEDC_INTR_DISABLE,
                                       .timer_sel = LEDC_TIMER,
                                       .duty = 0,
                                       .hpoint = 0,
                                       .flags = {0}};

  ledc_channel_config_t green_channel = {.gpio_num = GREEN_PIN,
                                         .speed_mode = LEDC_MODE,
                                         .channel = GREEN_CHANNEL,
                                         .intr_type = LEDC_INTR_DISABLE,
                                         .timer_sel = LEDC_TIMER,
                                         .duty = 0,
                                         .hpoint = 0,
                                         .flags = {0}};

  ledc_channel_config_t blue_channel = {.gpio_num = BLUE_PIN,
                                        .speed_mode = LEDC_MODE,
                                        .channel = BLUE_CHANNEL,
                                        .intr_type = LEDC_INTR_DISABLE,
                                        .timer_sel = LEDC_TIMER,
                                        .duty = 0,
                                        .hpoint = 0,
                                        .flags = {0}};

  if (ledc_channel_config(&red_channel) != ESP_OK) {
    ESP_LOGE(TAG, "Failed to configure red channel");
    return false;
  }

  if (ledc_channel_config(&green_channel) != ESP_OK) {
    ESP_LOGE(TAG, "Failed to configure green channel");
    return false;
  }

  if (ledc_channel_config(&blue_channel) != ESP_OK) {
    ESP_LOGE(TAG, "Failed to configure blue channel");
    return false;
  }

  set_rgb(0, 0, 0);
  return true;
}

void RGBLed::rgb_led_task(void *arg) {
  auto *rgb_led = static_cast<RGBLed *>(arg);
  rgb_led->task_function();

  vTaskDelete(nullptr);
  ESP_LOGI(TAG, "RGB LED task deleted itself");
}

void RGBLed::task_function() {
  Pattern current_pattern_local = Pattern::OFF;
  uint32_t delay_ms = 1000;

  while (_running) {
    // Get current pattern
    if (_pattern_mutex != NULL &&
        xSemaphoreTake(_pattern_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
      current_pattern_local = _current_pattern;
      xSemaphoreGive(_pattern_mutex);
    } else {
      ESP_LOGW(TAG, "Failed to acquire pattern mutex");
      vTaskDelay(pdMS_TO_TICKS(100));
      continue;
    }

    switch (current_pattern_local) {
    case Pattern::OFF:
      set_color(current_pattern_local);
      delay_ms = 1000;
      break;

    case Pattern::ON:
      set_color(current_pattern_local);
      delay_ms = 1000;
      break;

    case Pattern::NO_QR_CODE:
      set_color(current_pattern_local);
      delay_ms = 1000;
      break;

    case Pattern::STATIC_CONFIG_SAVED:
      set_color(current_pattern_local);
      delay_ms = 500;
      break;

    case Pattern::MQTT_CONNECTED:
      set_color(current_pattern_local);
      delay_ms = 500;
      break;

    case Pattern::ERROR_BLINK:
      blink_led(current_pattern_local);
      delay_ms = 250;
      break;
    }

    vTaskDelay(pdMS_TO_TICKS(delay_ms));
  }
}