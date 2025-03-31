idf_component_register(SRCS "rgb_led.cpp" "led.cpp"
                    INCLUDE_DIRS "include"
                    REQUIRES driver esp_driver_ledc)
