idf_component_register(SRCS "sensors.cpp" "cpu_temp.cpp" "charge_current.cpp" "battery_temp.cpp"
                            "light_sensor.cpp" "battery_manager.cpp" "battery_charge.cpp"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES utilities
                    REQUIRES driver communication)
