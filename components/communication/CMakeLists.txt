idf_component_register(SRCS "wifi.cpp" "mqtt.cpp"
                       INCLUDE_DIRS "include"
                       REQUIRES esp_wifi mqtt esp_event esp_netif error_handler)
