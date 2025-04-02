idf_component_register(SRCS "wifi.cpp" "mqtt.cpp" "http_client.cpp" "i2c_manager.cpp"
                       INCLUDE_DIRS "include"
                       PRIV_REQUIRES utilities storage
                       REQUIRES esp_wifi mqtt esp_event esp_netif esp_http_client esp_driver_i2c)
