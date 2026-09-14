set(PROJ_SRC ${CMAKE_CURRENT_LIST_DIR})

idf_component_register(
    SRCS "Clock.cpp"
    INCLUDE_DIRS ${PROJ_SRC}
    REQUIRES brookesia_core rtc_service
    WHOLE_ARCHIVE
)

set_source_files_properties(
    "Clock.cpp"
    PROPERTIES COMPILE_FLAGS "-Wno-missing-field-initializers"
)
