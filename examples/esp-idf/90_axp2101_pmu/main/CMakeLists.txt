idf_component_register(SRCS "main.cpp"
                    "port_axp2101.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES board_variant XPowersLib esp_driver_i2c)
