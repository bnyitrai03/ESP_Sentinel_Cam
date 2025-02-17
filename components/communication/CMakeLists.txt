idf_component_register(SRCS "wifi.cpp" "mqtt.cpp"
                       INCLUDE_DIRS "include"
                       PRIV_REQUIRES error_handler storage
                       REQUIRES esp_wifi mqtt esp_event esp_netif)
