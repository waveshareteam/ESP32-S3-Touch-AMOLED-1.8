idf_component_register(SRCS "pcf85063_rtc.c"
                    INCLUDE_DIRS "."
                    REQUIRES board_variant pcf85063a esp_driver_i2c)
