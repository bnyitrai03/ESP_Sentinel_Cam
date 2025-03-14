idf_component_register(SRCS "src/main.cpp" "src/qr_reader_app.cpp" "src/camera_app.cpp"
                       PRIV_INCLUDE_DIRS "include"
                       PRIV_REQUIRES utilities storage camera communication sensors mytime esp_psram qr led button event)
