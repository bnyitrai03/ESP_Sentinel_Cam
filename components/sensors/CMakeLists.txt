idf_component_register(SRCS "sensors.cpp" "cpu_temp.cpp" "myadc.cpp" "light_sensor.cpp" "battery_charge.cpp"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES utilities
                    REQUIRES esp_adc driver)
