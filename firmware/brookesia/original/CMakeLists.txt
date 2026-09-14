cmake_minimum_required(VERSION 3.16)

set(IDF_TARGET esp32s3)
set(SDKCONFIG_DEFAULTS "${CMAKE_CURRENT_LIST_DIR}/../sdkconfig.defaults")
set(EXTRA_COMPONENT_DIRS "${CMAKE_CURRENT_LIST_DIR}/../components")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(esp32-s3-touch-amoled-1-8-brookesia-original)

# mcp-c-sdk 2.0.1 triggers GCC 14's conservative bounded-strncpy warning.
# Keep the managed component unchanged and relax only that warning on its own
# target, matching the ESP-IDF 5.5 Brookesia reference build.
if(TARGET __idf_espressif__mcp-c-sdk AND CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(__idf_espressif__mcp-c-sdk PRIVATE
        -Wno-error=stringop-truncation
    )
endif()
