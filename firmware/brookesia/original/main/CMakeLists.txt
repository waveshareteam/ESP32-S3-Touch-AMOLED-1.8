idf_component_register(
    SRCS "../../main/main.cpp"
    INCLUDE_DIRS "../../main"
    REQUIRES
        brookesia_app_calculator
        bsp_extra
        Clock
        draw
        Gallery
        Gravitysphere
        MusicPlayer
        Recorder
        rtc_service
        Settings
        SpecAnalyzer
        VideoPlayer
        XiaozhiApp
        chat_history
        storage_service
        system_status
)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-missing-field-initializers)

set(STORAGE_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../spiffs")
set(STORAGE_STAGING_DIR "${CMAKE_BINARY_DIR}/storage_spiffs")

idf_build_get_property(build_components BUILD_COMPONENTS)
set(XIAOZHI_FONTS_DIR "")
foreach(component IN LISTS build_components)
    if(component MATCHES "xiaozhi-fonts")
        idf_component_get_property(XIAOZHI_FONTS_DIR ${component} COMPONENT_DIR)
        break()
    endif()
endforeach()

if(NOT XIAOZHI_FONTS_DIR)
    message(FATAL_ERROR "xiaozhi-fonts component was not resolved")
endif()

set(XIAOZHI_COMMON_FONT
    "${XIAOZHI_FONTS_DIR}/cbin/font_puhui_common_30_4.bin")
if(NOT EXISTS "${XIAOZHI_COMMON_FONT}")
    message(FATAL_ERROR "xiaozhi-fonts is missing font_puhui_common_30_4.bin")
endif()

# Staging is intentionally unconditional. A stamp-gated copy can silently keep a
# stale staging directory, which packages an out-of-date SPIFFS image with no
# build error at all: bundled media then simply never appears on the device.
# Copying a few megabytes on every build is the cheap price for that guarantee.
add_custom_target(storage_spiffs_stage
    COMMAND "${CMAKE_COMMAND}" -E rm -rf "${STORAGE_STAGING_DIR}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${STORAGE_STAGING_DIR}"
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
        "${STORAGE_SOURCE_DIR}" "${STORAGE_STAGING_DIR}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${STORAGE_STAGING_DIR}/xiaozhi"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "${XIAOZHI_COMMON_FONT}" "${STORAGE_STAGING_DIR}/xiaozhi/font.bin"
    COMMENT "Staging Original-profile Brookesia SPIFFS resources"
    VERBATIM
)

spiffs_create_partition_image(
    storage "${STORAGE_STAGING_DIR}"
    FLASH_IN_PROJECT
    DEPENDS storage_spiffs_stage
)
