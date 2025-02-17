#include "config.h"
#include "error_handler.h"
#include "esp_log.h"
#include "json_converters.h"
#include "storage.h"

constexpr auto *TAG = "Config";

std::vector<TimingConfig> Config::_timing;
std::vector<TimingConfig>::iterator Config::_active;
char Config::uuid[40] = {0};

Config::Config() {
  Config::load();
  set_active_config();
}

void Config::load() {
  char config[4000];
  esp_err_t err = Storage::read("static_config", config, sizeof(config));
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Error (%s) reading static config from storage!",
             esp_err_to_name(err));
    restart();
  }

  // .. todo:: Add error handling for JSON parsing
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, config);
  if (error) {
    ESP_LOGE(TAG, "Error parsing JSON: %s", error.c_str());
    restart();
  }
  if (strlcpy(uuid, doc["uuid"], sizeof(uuid)) == 0) {
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
}

void Config::set_active_config() {
  char timestamp[9] = {0};
  Time::get_local_time(timestamp, sizeof(timestamp));
  Time now(timestamp);

  for (auto it = _timing.begin(); it != _timing.end(); ++it) {
    if (now >= it->start && now <= it->end) {
      _active = it;
      ESP_LOGI(TAG,
               "Active config: %02d:%02d:%02d - %02d:%02d:%02d, period: %d",
               it->start.get_hours(), it->start.get_minutes(),
               it->start.get_seconds(), it->end.get_hours(),
               it->end.get_minutes(), it->end.get_seconds(), it->period);
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

void Config::update(std::string &config) {
  // .. todo:: Add error handling for JSON parsing
  esp_err_t err = Storage::write("static_config", config.c_str());
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Error (%s) writing static_config to storage!",
             esp_err_to_name(err));
    restart();
  }
}