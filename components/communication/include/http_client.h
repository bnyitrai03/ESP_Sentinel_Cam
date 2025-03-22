#pragma once

#include "esp_http_client.h"
#include <ArduinoJson.h>
#include <string>

/**
 * @brief
 * HTTP client class for sending GET request
 *
 */
class HTTPClient {
public:
  /**
   * @brief Sends a GET request to the specified URL and returns the static
   * config as a JSON
   *
   * @param url The URL to send the request to
   * @param response JSON document to store the response
   * @return esp_err_t
   *
   *                  - ESP_OK on success
   *
   *                  - ESP_FAIL on failure
   */
  static esp_err_t get_config(const char *url, JsonDocument &response);

private:
  /**
   * @brief Event handler for HTTP events
   */
  static esp_err_t event_handler(esp_http_client_event_t *evt);

  static constexpr uint32_t MAX_HTTP_OUTPUT_BUFFER = 1024;
};