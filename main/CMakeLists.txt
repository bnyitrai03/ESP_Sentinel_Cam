idf_component_register(SRCS take_picture.c
                        PRIV_INCLUDE_DIRS .
                        PRIV_REQUIRES nvs_flash esp_camera esp_http_server esp_wifi)
