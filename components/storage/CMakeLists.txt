idf_component_register(SRCS "storage.cpp"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES error_handler
                    REQUIRES nvs_flash)
