cmake_minimum_required(VERSION 3.16)

# Force linux target build
set(IDF_TARGET linux)

# Extra components: linux_compat layers and our mock tcp_transport
set(EXTRA_COMPONENT_DIRS
    ${CMAKE_CURRENT_LIST_DIR}/components
    ${CMAKE_CURRENT_LIST_DIR}/../../deps/protocols/common_components/linux_compat/freertos
    ${CMAKE_CURRENT_LIST_DIR}/../../deps/protocols/common_components/linux_compat/esp_timer
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
idf_build_set_property(MINIMAL_BUILD ON)

project(mqtt_host)

