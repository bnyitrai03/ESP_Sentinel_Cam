#ifndef MYTIME_H
#define MYTIME_H
#include "esp_err.h"
#include <string>

/**
 * @brief Manages time operations.
 */
class Time {
public:
  /**
   * @brief Default constructor for Time class
   *
   * Initializes time with invalid values.
   */
  Time() : _hours(-1), _minutes(-1), _seconds(-1) {}

  /**
   * @brief Constructor for Time class
   *
   * @param hour Hour value
   * @param minute Minute value
   * @param second Second value
   */
  Time(int hour, int minute, int second);

  /**
   * @brief Constructor for Time class
   *
   * @param timestamp Timestamp string in the format HH:MM:SS
   */
  Time(const std::string &timestamp);

  /**
   * @brief Gets the hour value
   *
   * @return Hour value
   */
  int get_hours() const { return _hours; }

  /**
   * @brief Gets the minute value
   *
   * @return Minute value
   */
  int get_minutes() const { return _minutes; }

  /**
   * @brief Gets the second value
   *
   * @return Second value
   */
  int get_seconds() const { return _seconds; }

  /**
   * @brief Less than operator for Time class.
   * @param other The other Time object to compare.
   * @return True if this time is less than the other time.
   */
  bool operator<(const Time &other) const {
    return toSeconds() < other.toSeconds();
  }
  /**
   * @brief Greater than operator for Time class.
   * @param other The other Time object to compare.
   * @return True if this time is greater than the other time.
   */
  bool operator>(const Time &other) const {
    return toSeconds() > other.toSeconds();
  }
  /**
   * @brief Equality operator for Time class.
   * @param other The other Time object to compare.
   * @return True if this time is equal to the other time.
   */
  bool operator==(const Time &other) const {
    return toSeconds() == other.toSeconds();
  }
  /**
   * @brief Less than or equal to operator for Time class.
   * @param other The other Time object to compare.
   * @return True if this time is less than or equal to the other time.
   */
  bool operator<=(const Time &other) const {
    return toSeconds() <= other.toSeconds();
  }
  /**
   * @brief Greater than or equal to operator for Time class.
   * @param other The other Time object to compare.
   * @return True if this time is greater than or equal to the other time.
   */
  bool operator>=(const Time &other) const {
    return toSeconds() >= other.toSeconds();
  }

  /**
   * @brief Gets the current utc time in %H:%M:%S format
   *
   * @param timestamp
   *    - The buffer to store the current utc time
   * @param size
   *   - The size of the buffer
   *
   * @return
   *     - ESP_OK on success
   *     - ESP_FAIL on error
   */
  static esp_err_t get_time(char *timestamp, uint32_t size);

  /**
   * @brief Gets the current UTC timestamp in %Y-%m-%dT%H:%M:%SZ format
   *
   * @param timestamp
   *    - The buffer to store the current UTC datetime
   * @param size
   *   - The size of the buffer
   *
   * @return
   *     - ESP_OK on success
   *     - ESP_FAIL on error
   */
  static esp_err_t get_date(char *timestamp, uint32_t size);

private:
  int _hours;
  int _minutes;
  int _seconds;

  /**
   * @brief Converts time to seconds
   *
   * @return Time in seconds
   */
  int toSeconds() const { return _hours * 3600 + _minutes * 60 + _seconds; }
};
#endif // MYTIME_H