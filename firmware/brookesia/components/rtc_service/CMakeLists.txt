idf_component_register(
    SRCS "rtc_service.c"
    INCLUDE_DIRS "include"
    REQUIRES
        esp32_s3_touch_amoled_1_8
        esp_driver_i2c
        waveshare__pcf85063a
)
