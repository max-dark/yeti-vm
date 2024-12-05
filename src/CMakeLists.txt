cmake_minimum_required(VERSION 3.20)

project(RiscVM LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(shared INTERFACE)
target_include_directories(
    shared
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
)
target_sources(
    shared
    INTERFACE
        stdlib.hxx
)

set(APP_NAME risc-vm)

set(APP_SOURCES
    main.cxx
)

set(LIB_NAME rvm)
set(LIB_HEADERS
    vm_base_types.hxx
    vm_utility.hxx
    vm_opcode.hxx
)
set(LIB_SOURCES
    vm_base_types.cxx
    vm_utility.cxx
    vm_opcode.cxx
)
add_library(${LIB_NAME} STATIC)
target_sources(
    ${LIB_NAME}
    PRIVATE
        ${LIB_SOURCES}
    PUBLIC
        ${LIB_HEADERS}
)
target_link_libraries(
    ${LIB_NAME}
    PUBLIC
        shared
)

add_executable(${APP_NAME})
target_sources(
    ${APP_NAME}
    PRIVATE
        ${APP_SOURCES}
)
target_link_libraries(${APP_NAME}
    PRIVATE
        ${LIB_NAME}
        shared
)
