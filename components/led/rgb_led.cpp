#include "rgb_led.h"
#include "driver/rtc_io.h"
#include "esp_log.h"
#include "esp_system.h"

constexpr auto *TAG = "RGB_LED";

constexpr ledc_timer_t LEDC_TIMER = LEDC_TIMER_0;
constexpr ledc_mode_t LEDC_MODE = LEDC_LOW_SPEED_MODE;
constexpr ledc_channel_t RED_CHANNEL = LEDC_CHANNEL_0;
constexpr ledc_channel_t GREEN_CHANNEL = LEDC_CHANNEL_1;
constexpr ledc_channel_t BLUE_CHANNEL = LEDC_CHANNEL_2;
constexpr uint32_t LEDC_FREQUENCY = 5000; // 5kHz PWM frequency

RGBLed::Pattern RGBLed::current_pattern = RGBLed::Pattern::OFF;
bool RGBLed::led_state = false;
SemaphoreHandle_t RGBLed::pattern_mutex = xSemaphoreCreateMutex();
SemaphoreHandle_t RGBLed::led_mutex = xSemaphoreCreateMutex();

const RGBLed::RGBColor RGBLed::PATTERN_COLORS[] = {
    {0, 0, 0},       // OFF - No color
    {255, 255, 255}, // ON - White
    {0, 0, 255},     // NO_QR_CODE - Blue
    {0, 255, 0},     // STATIC_CONFIG_SAVED - Green
    {255, 165, 0},   // MQTT_CONNECTED - Orange
    {255, 0, 0}      // ERROR_BLINK - Red
};

RGBLed::RGBLed() {

  // Reset the LED states if the device is waking up from deep sleep
  if (esp_reset_reason() == ESP_RST_DEEPSLEEP) {
    rtc_gpio_hold_dis(RED_PIN);
    rtc_gpio_hold_dis(GREEN_PIN);
    rtc_gpio_hold_dis(BLUE_PIN);
  }

  // Initialize LEDC for PWM control
  ledc_timer_config_t ledc_timer = {.speed_mode = LEDC_MODE,
                                    .duty_resolution = LEDC_TIMER_8_BIT,
                                    .timer_num = LEDC_TIMER,
                                    .freq_hz = LEDC_FREQUENCY,
                                    .clk_cfg = LEDC_AUTO_CLK};
  if (ledc_timer_config(&ledc_timer) != ESP_OK) {
    esp_restart();
  }

  // Configure channels for each color
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
    esp_restart();
  }
  if (ledc_channel_config(&green_channel) != ESP_OK) {
    esp_restart();
  }
  if (ledc_channel_config(&blue_channel) != ESP_OK) {
    esp_restart();
  }

  running = true;
  auto res = xTaskCreate(rgb_led_task, "rgb_led_task", 3000, this, 2, nullptr);
  if (res != pdPASS) {
    ESP_LOGE(TAG, "Failed to create RGB LED task");
    esp_restart();
  }
}

RGBLed::~RGBLed() { running = false; }

void RGBLed::stop() {
  running = false;
  ESP_LOGI(TAG, "RGB LED task stopped");
}

void RGBLed::set_pattern(Pattern pattern) {
  if (pattern_mutex == NULL) {
    ESP_LOGE(TAG, "Pattern mutex not initialized");
    return;
  }
  if (xSemaphoreTake(pattern_mutex, pdMS_TO_TICKS(500)) == pdTRUE) {
    current_pattern = pattern;
    xSemaphoreGive(pattern_mutex);
  }
}

void RGBLed::set_rgb(uint8_t r, uint8_t g, uint8_t b) {

  if (xSemaphoreTake(led_mutex, pdMS_TO_TICKS(500)) == pdTRUE) {
    ledc_set_duty(LEDC_MODE, RED_CHANNEL, r);
    ledc_update_duty(LEDC_MODE, RED_CHANNEL);

    ledc_set_duty(LEDC_MODE, GREEN_CHANNEL, g);
    ledc_update_duty(LEDC_MODE, GREEN_CHANNEL);

    ledc_set_duty(LEDC_MODE, BLUE_CHANNEL, b);
    ledc_update_duty(LEDC_MODE, BLUE_CHANNEL);

    xSemaphoreGive(pattern_mutex);
  }
}

void RGBLed::set_color(Pattern pattern) {
  // Get pattern index to access the right color
  int pattern_index = static_cast<int>(pattern);

  // Set the color for this pattern
  const RGBColor &color = PATTERN_COLORS[pattern_index];
  set_rgb(color.r, color.g, color.b);
  led_state = true;
}

bool RGBLed::blink_led(Pattern pattern) {
  // Toggle LED state
  if (led_state) {
    set_color(pattern);
    led_state = false;
  } else {
    set_color(Pattern::OFF);
    led_state = true;
  }

  return led_state;
}

void RGBLed::rgb_led_task(void *arg) {
  // Give context to the task
  auto *rgb_led = static_cast<RGBLed *>(arg);
  rgb_led->task_function();
  // Clean up
  vTaskDelete(nullptr);
  ESP_LOGI(TAG, "RGB LED task deleted itself");
}

void RGBLed::task_function() {
  Pattern current_pattern_local = Pattern::ERROR_BLINK;
  uint32_t delay_ms = 1000;

  while (running) {
    // Get current pattern
    if (pattern_mutex != NULL &&
        xSemaphoreTake(pattern_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
      current_pattern_local = current_pattern;
      xSemaphoreGive(pattern_mutex);
    } else {
      ESP_LOGW(TAG, "Failed to acquire pattern mutex");
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

    vTaskDelay(delay_ms);
  }
}