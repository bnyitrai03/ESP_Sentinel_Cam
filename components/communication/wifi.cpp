#include "wifi.h"
#include "error_handler.h"
#include "esp_log.h"
#include "esp_netif_sntp.h"
#include "esp_sntp.h"
#include "esp_wifi.h"
#include "string.h"
#include <cstring>

constexpr auto *TAG = "WiFi";

EventGroupHandle_t Wifi::_wifi_event_group = nullptr;
bool Wifi::_connected = false;

Wifi::Wifi() { _wifi_event_group = xEventGroupCreate(); }

Wifi::~Wifi() { vEventGroupDelete(_wifi_event_group); }

void Wifi::init() {
  esp_netif_init();
  esp_event_loop_create_default();
  esp_netif_create_default_wifi_sta();
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  if (esp_wifi_init(&cfg) != ESP_OK) {
    ESP_LOGE(TAG, "Failed to initialize the WiFi stack!");
    restart();
  }

  esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                      &Wifi::eventHandler, NULL, NULL);
  esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                      &Wifi::eventHandler, NULL, NULL);
  set_wifi_deinit_callback([]() {
    _connected = false;
    esp_wifi_disconnect();
    esp_wifi_stop();
    esp_wifi_deinit();
  });
}

void Wifi::connect(const std::string &ssid, const std::string &password) {
  wifi_config_t wifi_config = {};
  strncpy((char *)wifi_config.sta.ssid, ssid.c_str(),
          sizeof(wifi_config.sta.ssid));
  strncpy((char *)wifi_config.sta.password, password.c_str(),
          sizeof(wifi_config.sta.password));

  esp_wifi_set_mode(WIFI_MODE_STA);
  esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
  if (esp_wifi_start() != ESP_OK) {
    ESP_LOGE(TAG, "Failed to start WiFi station!");
    restart();
  }

  ESP_LOGI(TAG, "Waiting for WiFi connection...");

  EventBits_t bits = xEventGroupWaitBits(_wifi_event_group, WIFI_CONNECTED_BIT,
                                         pdFALSE, pdFALSE, portMAX_DELAY);

  if (bits & WIFI_CONNECTED_BIT) {
    _connected = true;
    ESP_LOGI(TAG, "WiFi Connected, syncing NTP...");
  }

  // Set timezone to Budapest
  setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
  tzset();
  // Configure SNTP
  esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
  esp_netif_sntp_init(&config);
  if (esp_netif_sntp_sync_wait(pdMS_TO_TICKS(15000)) != ESP_OK) {
    ESP_LOGE(TAG, "Failed to configure NTP server!");
    restart();
  }
}

void Wifi::eventHandler(void *arg, esp_event_base_t event_base,
                        int32_t event_id, void *event_data) {
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    esp_wifi_connect();
  } else if (event_base == WIFI_EVENT &&
             event_id == WIFI_EVENT_STA_DISCONNECTED && _connected) {
    ESP_LOGI(TAG, "Retry connecting to the AP");
    esp_wifi_connect();
    xEventGroupClearBits(_wifi_event_group, WIFI_CONNECTED_BIT);
  } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    ESP_LOGI(TAG, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
    xEventGroupSetBits(_wifi_event_group, WIFI_CONNECTED_BIT);
  }
}