idf_component_register(SRCS "error_handler.cpp" "mysleep.cpp"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES esp_timer
                    REQUIRES mytime)
