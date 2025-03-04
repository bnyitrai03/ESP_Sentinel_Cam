idf_component_register(SRCS "qr_decoder.cpp"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES esp32-camera utilities storage
                    REQUIRES quirc)
