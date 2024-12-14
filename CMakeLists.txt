cmake_minimum_required(VERSION 3.25)

project(Yeti LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_subdirectory(examples)

add_subdirectory(src)
add_subdirectory(tools)
add_subdirectory(tests)
