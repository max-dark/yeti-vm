cmake_minimum_required(VERSION 3.20)

project(YetiTools LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

set(APP_NAME yeti-vm)
set(APP_SOURCES
    yeti_main.cxx
)
set(APP_DEPS
    vm_basic
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

