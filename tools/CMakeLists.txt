cmake_minimum_required(VERSION 3.20)

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
        rvm
)
