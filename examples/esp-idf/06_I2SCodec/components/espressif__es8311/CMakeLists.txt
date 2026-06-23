idf_component_register(
    SRCS "es8311.c"
    INCLUDE_DIRS "include"
    PRIV_INCLUDE_DIRS "priv_include"
    REQUIRES esp_driver_i2c esp_driver_gpio freertos
)
