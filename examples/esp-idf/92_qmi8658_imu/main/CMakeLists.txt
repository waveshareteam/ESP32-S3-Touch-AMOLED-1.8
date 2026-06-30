idf_component_register(SRCS "qmi8658_imu.c"
                    INCLUDE_DIRS "."
                    REQUIRES board_variant qmi8658 esp_driver_i2c)
