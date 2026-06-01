idf_component_register(SRCS "main.cpp"
                    "port_axp2101.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES board_variant esp32_s3_touch_amoled_1_8 XPowersLib esp_driver_i2c)
