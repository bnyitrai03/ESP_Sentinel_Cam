#pragma once

#include "esp_event.h"
#include "freertos/event_groups.h"
#include <string>

/**
 * @brief Handles WiFi connection
 */
class Wifi {
public:
  /**
   * @brief Constructor for Wifi class
   */
  Wifi();

  /**
   * @brief Connects to the WiFi
   */
  void connect();

  /**
   * @brief Syncs the time with a remote NTP server
   */
  void sync_time();

private:
  /**
   * @brief Event handler for WiFi events
   *
   * @param arg User-defined argument
   * @param event_base Event base
   * @param event_id Event ID
   * @param event_data Event data
   */
  static void eventHandler(void *arg, esp_event_base_t event_base,
                           int32_t event_id, void *event_data);

  static EventGroupHandle_t _wifi_event_group;
  static const int WIFI_CONNECTED_BIT = BIT0;
  static bool _connected;
};