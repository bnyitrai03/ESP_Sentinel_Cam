idf_component_register(SRCS "src/main.cpp"
                       PRIV_INCLUDE_DIRS "include"
                       PRIV_REQUIRES error_handler storage camera communication mytime nvs_flash esp_psram)
