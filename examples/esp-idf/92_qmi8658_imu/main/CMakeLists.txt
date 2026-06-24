idf_component_register(SRCS "qmi8658c.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES board_variant SensorLib esp_driver_i2c)
