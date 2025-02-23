#pragma once

#include "mytime.h"
#include <ArduinoJson.h>

namespace ArduinoJson {
template <> struct Converter<Time> {
  static bool fromJson(JsonVariantConst src, Time &dst) {
    if (!src.is<const char *>()) {
      return false;
    }
    dst = Time(src.as<const char *>());
    return true;
  }

  static JsonVariantConst toJson(const Time &src, JsonVariant dst) {
    char buffer[9]; // HH:MM:SS
    snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", src.get_hours(),
             src.get_minutes(), src.get_seconds());
    dst.set(buffer);
    return dst;
  }
};
} // namespace ArduinoJson