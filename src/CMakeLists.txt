cmake_minimum_required(VERSION 3.20)

project(YetiVM LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(yeti_vm_shared INTERFACE)
target_include_directories(
    yeti_vm_shared
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
)
target_sources(
    yeti_vm_shared
    INTERFACE
        vm_stdlib.hxx
)
add_library(YetiVM::shared ALIAS yeti_vm_shared)

set(LIB_NAME yeti_vm_runtime)
set(LIB_HEADERS
    vm_base_types.hxx
    vm_utility.hxx
    vm_opcode.hxx
    vm_handler.hxx
    vm_interface.hxx
    vm_memory.hxx
    vm_syscall.hxx
    vm_handlers_rv32i.hxx
    vm_handlers_rv32m.hxx
)
set(LIB_SOURCES
    vm_base_types.cxx
    vm_utility.cxx
    vm_opcode.cxx
    vm_handler.cxx
    vm_interface.cxx
    vm_memory.cxx
    vm_syscall.cxx
    vm_handlers_rv32i.cxx
    vm_handlers_rv32m.cxx
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
        YetiVM::shared
)
add_library(YetiVM::runtime ALIAS ${LIB_NAME})

set(LIB_BASIC_VM yeti_vm_basic)
add_library(${LIB_BASIC_VM} STATIC)
target_sources(
    ${LIB_BASIC_VM}
    PUBLIC
        vm_basic.hxx
    PRIVATE
        vm_basic.cxx
)
target_link_libraries(
    ${LIB_BASIC_VM}
    PUBLIC
        YetiVM::runtime
)
add_library(YetiVM::basic_vm ALIAS ${LIB_BASIC_VM})
