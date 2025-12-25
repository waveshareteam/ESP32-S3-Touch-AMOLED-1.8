idf_component_register(
    SRCS "example_qspi_with_ram.c"
    INCLUDE_DIRS ".")

set_source_files_properties(
    ${LV_DEMOS_SOURCES}
    PROPERTIES COMPILE_OPTIONS
    -DLV_LVGL_H_INCLUDE_SIMPLE)
