idf_component_register(
    SRCS
        "esp_lcd_panel_io_3wire_spi.c"
    INCLUDE_DIRS
        "include"
    PRIV_REQUIRES
        "esp_driver_gpio"
        "esp_driver_spi"
    REQUIRES
        "esp_lcd")

include(package_manager)
cu_pkg_define_version(${CMAKE_CURRENT_LIST_DIR})
