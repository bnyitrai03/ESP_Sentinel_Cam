#ifndef MYTIME_H
#define MYTIME_H
#include "esp_err.h"
#include <string>

class Time {
public:
  Time() : _hours(-1), _minutes(-1), _seconds(-1) {} /*!< Use invalid values*/
  Time(int hour, int minute, int second);
  Time(const std::string &timestamp);

  int get_hours() const { return _hours; }
  int get_minutes() const { return _minutes; }
  int get_seconds() const { return _seconds; }

  bool operator<(const Time &other) const {
    return toSeconds() < other.toSeconds();
  }
  bool operator>(const Time &other) const {
    return toSeconds() > other.toSeconds();
  }
  bool operator==(const Time &other) const {
    return toSeconds() == other.toSeconds();
  }
  bool operator<=(const Time &other) const {
    return toSeconds() <= other.toSeconds();
  }
  bool operator>=(const Time &other) const {
    return toSeconds() >= other.toSeconds();
  }

  /**
   * @brief Gets the current local time
   *
   * @param timestamp
   *    - The buffer to store the current local time
   * @param size
   *   - The size of the buffer
   *
   * @return
   *     - The current local time in the format HH:MM:SS
   */
  static esp_err_t get_local_time(char *timestamp, uint32_t size);
  /**
   * @brief Gets the current UTC timestamp
   *
   * @param timestamp
   *    - The buffer to store the current local time
   * @param size
   *   - The size of the buffer
   *
   * @return
   *     - The current UTC time in the format Y-m-dTH:M:SZ
   */
  static esp_err_t get_utc_timestamp(char *timestamp, uint32_t size);

private:
  int _hours;
  int _minutes;
  int _seconds;

  int toSeconds() const { return _hours * 3600 + _minutes * 60 + _seconds; }
};
#endif // MYTIME_H