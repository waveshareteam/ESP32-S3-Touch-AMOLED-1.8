idf_component_register(SRCS "gpio_io_main.c" INCLUDE_DIRS "." REQUIRES esp_driver_gpio)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-error=shift-count-negative)
