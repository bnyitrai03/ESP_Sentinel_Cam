idf_component_register(SRCS "storage.cpp" "config.cpp"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES utilities event
                    REQUIRES nvs_flash mytime)
