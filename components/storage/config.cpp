#include "config.h"
#include "error_handler.h"
#include "esp_log.h"
#include "mysleep.h"
#include "storage.h"
#include <regex>

constexpr auto *TAG = "Config";

std::vector<TimingConfig> Config::_timing;
std::vector<TimingConfig>::iterator Config::_active;
char Config::_uuid[40] = {0};

void Config::load_config(JsonDocument &doc) {
  _timing.clear();

  if (strlcpy(_uuid, doc["uuid"], sizeof(_uuid)) == 0) {
    ESP_LOGE(TAG, "UUID not found in config!");
    restart();
  }
  JsonArray timingArray = doc["timing"];
  for (JsonVariant timing : timingArray) {
    TimingConfig tc;
    tc.period = timing["period"];
    if (!ArduinoJson::Converter<Time>::fromJson(timing["start"], tc.start)) {
      ESP_LOGE(TAG, "Invalid start time format in config!");
      restart();
    }
    if (!ArduinoJson::Converter<Time>::fromJson(timing["end"], tc.end)) {
      ESP_LOGE(TAG, "Invalid end time format in config!");
      restart();
    }
    _timing.push_back(tc);
  }

  Config::set_active_config();
}

void Config::load_from_storage() {
  char config[4000];
  esp_err_t err = Storage::read("dynamic_config", config, sizeof(config));
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Error (%s) reading dynamic config from storage!",
             esp_err_to_name(err));
    restart();
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, config);
  if (error) {
    ESP_LOGE(TAG, "Error parsing JSON: %s", error.c_str());
    restart();
  }

  if (Config::validate(doc)) {
    Config::load_config(doc);
  } else {
    ESP_LOGE(TAG, "Invalid config found in NVS!");
    restart();
  }
}

void Config::set_active_config() {
  char timestamp[9] = {0};
  Time::get_time(timestamp, sizeof(timestamp));
  Time now(timestamp);

  for (auto it = _timing.begin(); it != _timing.end(); ++it) {
    if (now >= it->start && now <= it->end) {
      _active = it;
      ESP_LOGI(TAG, "Active: %02d:%02d:%02d - %02d:%02d:%02d, period: %lld",
               it->start.get_hours(), it->start.get_minutes(),
               it->start.get_seconds(), it->end.get_hours(),
               it->end.get_minutes(), it->end.get_seconds(), it->period);
      if (it->period == -1) {
        ESP_LOGW(TAG, "Device is going to sleep");
        ESP_LOGW(TAG, "Device will wake up at %02d:%02d:%02d",
                 it->end.get_hours(), it->end.get_minutes(),
                 it->end.get_seconds());
        deinit_components();
        mysleep(it->end);
      }
      return;
    }
  }
}

TimingConfig Config::get_default_active_config() {
  TimingConfig tc;
  tc.period = 40;
  tc.start = Time(0, 0, 0);
  tc.end = Time(23, 59, 59);
  return tc;
}

TimingConfig Config::get_active_config() {
  if (_active != _timing.end()) {
    return *_active;
  } else {
    ESP_LOGE(TAG, "No active config found!");
    ESP_LOGE(TAG, "Setting default config,"
                  "working hours: 00:00 - 23:59, "
                  "period: 40");
    return get_default_active_config();
  }
}

bool Config::validate(JsonDocument &doc) {
  if (!doc["uuid"].is<std::string>() || doc["uuid"].as<std::string>().empty() ||
      doc["uuid"].as<std::string>().length() >= sizeof(_uuid)) {
    ESP_LOGE(TAG, "UUID is incorrect or empty");
    return false;
  }
  if (!doc["timing"].is<JsonArray>()) {
    ESP_LOGE(TAG, "Timing array is missing");
    return false;
  }

  // Validate time format: HH:MM:SS
  std::regex time_format("^([01]\\d|2[0-3]):([0-5]\\d):([0-5]\\d)$");
  JsonArray timingArray = doc["timing"];
  for (JsonVariant timing : timingArray) {
    if (!timing["period"].is<int>() || timing["period"].as<int>() < -1) {
      ESP_LOGE(TAG, "Period is missing or invalid in timing");
      return false;
    }
    if (!timing["start"].is<std::string>()) {
      ESP_LOGE(TAG, "Start time is missing");
      return false;
    }
    if (!std::regex_match(timing["start"].as<std::string>(), time_format)) {
      ESP_LOGE(TAG, "Start time format is invalid, expected HH:MM:SS");
      return false;
    }
    if (!timing["end"].is<std::string>()) {
      ESP_LOGE(TAG, "End time is missing");
      return false;
    }
    if (!std::regex_match(timing["end"].as<std::string>(), time_format)) {
      ESP_LOGE(TAG, "End time format is invalid, expected HH:MM:SS");
      return false;
    }
  }

  return true;
}