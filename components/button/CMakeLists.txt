idf_component_register(SRCS "button.cpp"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES utilities led event
                    REQUIRES driver)
