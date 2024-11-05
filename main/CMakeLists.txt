idf_component_register(SRCS main.c
                        PRIV_INCLUDE_DIRS .
                        PRIV_REQUIRES mbedtls nvs_flash esp_camera esp_http_server esp_wifi mqtt)
