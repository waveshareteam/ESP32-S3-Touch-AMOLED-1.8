idf_component_register(SRCS "qmi8658c.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES board_variant esp32_s3_touch_amoled_1_8 SensorLib esp_driver_i2c)
