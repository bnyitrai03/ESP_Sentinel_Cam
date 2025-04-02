#include "http_client.h"
#include "esp_log.h"
#include <memory>

constexpr auto *TAG = "HTTPClient";

esp_err_t HTTPClient::get_config(const char *url, JsonDocument &response) {
  esp_http_client_config_t config = {
      .url = url,
      .cert_pem = NULL,
      .timeout_ms = 15000,
      .event_handler = event_handler,
      .transport_type = HTTP_TRANSPORT_OVER_SSL,
      .skip_cert_common_name_check = true,
  };

  esp_http_client_handle_t client = esp_http_client_init(&config);
  if (!client) {
    ESP_LOGE(TAG, "Failed to initialize HTTPS client");
    return ESP_FAIL;
  }

  std::unique_ptr<char[]> buffer(new char[MAX_HTTP_OUTPUT_BUFFER]);

  esp_err_t err = esp_http_client_open(client, 0);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to open HTTPS connection: %s", esp_err_to_name(err));
    esp_http_client_cleanup(client);
    return ESP_FAIL;
  }

  int content_length = esp_http_client_fetch_headers(client);
  if (content_length < 0) {
    ESP_LOGE(TAG, "HTTPS client fetch headers failed");
    esp_http_client_cleanup(client);
    return ESP_FAIL;
  }

  int status_code = esp_http_client_get_status_code(client);
  if (status_code == HttpStatus_BadRequest) {
    ESP_LOGE(TAG, "Server returned HTTPS 400 Bad Request");
    ESP_LOGE(TAG, "The device is not registered with the server");
    esp_http_client_cleanup(client);
    return ESP_ERR_NOT_FOUND;
  }

  int read_len = esp_http_client_read_response(client, buffer.get(),
                                               MAX_HTTP_OUTPUT_BUFFER - 1);
  if (read_len <= 0) {
    ESP_LOGE(TAG, "Failed to read response");
    esp_http_client_cleanup(client);
    return ESP_FAIL;
  }

  buffer[read_len] = '\0';
  ESP_LOGI(TAG, "HTTPS GET response: %s", buffer.get());

  DeserializationError error = deserializeJson(response, buffer.get());
  if (error) {
    ESP_LOGE(TAG, "Failed to parse JSON: %s", error.c_str());
    esp_http_client_cleanup(client);
    return ESP_FAIL;
  }

  esp_http_client_cleanup(client);
  return ESP_OK;
}

esp_err_t HTTPClient::event_handler(esp_http_client_event_t *evt) {
  switch (evt->event_id) {
  case HTTP_EVENT_ERROR:
    ESP_LOGE(TAG, "Event error: %s", static_cast<const char *>(evt->data));
    break;
  default:
    break;
  }
  return ESP_OK;
}