idf_component_register(SRCS "camera.cpp"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES utilities storage
                    REQUIRES esp32-camera)
