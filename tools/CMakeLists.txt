cmake_minimum_required(VERSION 3.25)

project(YetiTools LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

set(HEX_VIEWER view-hex)
add_executable(${HEX_VIEWER})
target_sources(
    ${HEX_VIEWER}
    PRIVATE
        view_hex.cxx
)

target_link_libraries(
    ${HEX_VIEWER}
    PRIVATE
        YetiVM::runtime
)

set(APP_NAME yeti-vm)
set(APP_SOURCES
    yeti_main.cxx
)
set(APP_DEPS
    YetiVM::basic_vm
)
add_executable(${APP_NAME})
target_sources(
    ${APP_NAME}
    PRIVATE
        ${APP_SOURCES}
)
target_link_libraries(
    ${APP_NAME}
    PRIVATE
        ${APP_DEPS}
)

add_executable(make_dummy make_dummy.cxx)
