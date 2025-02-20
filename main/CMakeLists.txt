idf_component_register(SRCS "src/main.cpp" "src/qr_reader.cpp" "src/camera_app.cpp"
                       PRIV_INCLUDE_DIRS "include"
                       PRIV_REQUIRES error_handler storage camera communication mytime esp_psram)
