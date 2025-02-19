#include "mytime.h"
#include "error_handler.h"
#include "esp_log.h"
#include <iomanip>
#include <sstream>

constexpr auto *TAG = "Time";

Time::Time(int hour, int minute, int second) {
  if (second >= 60 || minute >= 60 || hour >= 24 || second < 0 || minute < 0 ||
      hour < 0) {
    ESP_LOGE(TAG, "Invalid timeformat in dynamic config");
    restart();
  }
  _hours = hour;
  _minutes = minute;
  _seconds = second;
}

Time::Time(const std::string &timestamp) {
  std::istringstream ss(timestamp);
  char delimiter;
  if (!(ss >> _hours >> delimiter >> _minutes >> delimiter >> _seconds) ||
      delimiter != ':' || _hours >= 24 || _minutes >= 60 || _seconds >= 60 ||
      _hours < 0 || _minutes < 0 || _seconds < 0) {
    ESP_LOGE(TAG, "Invalid timestamp format: %s", timestamp.c_str());
    restart();
  }
}

esp_err_t Time::get_time(char *timestamp, uint32_t size) {
  time_t now;
  time(&now);
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  if (strftime(timestamp, size, "%H:%M:%S", &timeinfo) == 0) {
    ESP_LOGE(TAG, "Error formatting local time");
    return ESP_FAIL;
  }
  return ESP_OK;
}

esp_err_t Time::get_date(char *timestamp, uint32_t size) {
  time_t now;
  time(&now);
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  if (strftime(timestamp, size, "%Y-%m-%dT%H:%M:%SZ", &timeinfo) == 0) {
    ESP_LOGE(TAG, "Error formatting UTC timestamp");
    return ESP_FAIL;
  }
  return ESP_OK;
}