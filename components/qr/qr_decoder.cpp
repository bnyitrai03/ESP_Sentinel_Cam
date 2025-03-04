#include "qr_decoder.h"
#include "error_handler.h"
#include "esp_log.h"
#include "storage.h"
#include <cstring>

constexpr auto *TAG = "QRDecoder";

QRDecoder::QRDecoder(int width, int height) {
  _qr.reset(quirc_new());
  if (!_qr) {
    ESP_LOGE(TAG, "Failed to create QR code detector");
    restart();
  }

  if (quirc_resize(_qr.get(), width, height) < 0) {
    ESP_LOGE(TAG, "Failed to allocate QR buffer");
    restart();
  }
  ESP_LOGI(TAG, "QR decoder initialized with buffer %dx%d", width, height);
}

// See: https://github.com/dlbeer/quirc#library-use
bool QRDecoder::decode_frame(const camera_fb_t *frame) {
  if (!frame) {
    ESP_LOGE(TAG, "Frame is NULL");
    return false;
  }

  int width, height;
  uint8_t *qr_buf = quirc_begin(_qr.get(), &width, &height);
  if (!qr_buf) {
    ESP_LOGE(TAG, "Failed to get quirc buffer");
    return false;
  }

  if (frame->width != width || frame->height != height) {
    ESP_LOGE(TAG, "Frame size mismatch: expected %dx%d, got %dx%d", width,
             height, frame->width, frame->height);
    quirc_end(_qr.get());
    return false;
  }
  memcpy(qr_buf, frame->buf, frame->width * frame->height);
  quirc_end(_qr.get());

  int count = quirc_count(_qr.get());
  for (int i = 0; i < count; i++) {
    struct quirc_code code = {};
    struct quirc_data data = {};

    quirc_extract(_qr.get(), i, &code);
    quirc_flip(&code);

    ESP_LOGI(TAG, "Trying to decode QR code...");
    if (quirc_decode(&code, &data) == 0) {
      ESP_LOGI(TAG, "Decoded QR Code");
      save_decoded_data(reinterpret_cast<const char *>(data.payload),
                        data.payload_len);
      return true;
    } else {
      ESP_LOGE(TAG, "Failed to decode QR code");
    }
  }
  return false;
}

void QRDecoder::save_decoded_data(const char *data, uint32_t length) {
  if (!data || length == 0) {
    ESP_LOGE(TAG, "Decoded data is NULL");
    restart();
  }

  std::string qr_data(data, length);
  // Find positions of delimiters
  size_t first_delim = qr_data.find('|');
  size_t second_delim = qr_data.find('|', first_delim + 1);
  if (first_delim == std::string::npos || second_delim == std::string::npos) {
    ESP_LOGE(TAG, "Didn't find the delimiters in the QR code");
    restart();
  }

  // Extract the substrings
  std::string ssid = qr_data.substr(0, first_delim);
  std::string password =
      qr_data.substr(first_delim + 1, second_delim - first_delim - 1);
  std::string server = qr_data.substr(second_delim + 1);

  Storage::write("ssid", ssid);
  Storage::write("password", password);
  Storage::write("server_url", server);
  ESP_LOGI(TAG, "Saved WiFi SSID: %s", ssid.c_str());
  ESP_LOGI(TAG, "Saved WiFi Password: %s", password.c_str());
  ESP_LOGI(TAG, "Saved Server URL: %s", server.c_str());
}