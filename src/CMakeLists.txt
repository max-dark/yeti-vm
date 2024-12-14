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
        yeti-vm/vm_shared.hxx
)
add_library(YetiVM::shared ALIAS yeti_vm_shared)

set(LIB_NAME yeti_vm_runtime)
set(LIB_HEADERS
        yeti-vm/vm_base_types.hxx
        yeti-vm/vm_utility.hxx
        yeti-vm/vm_opcode.hxx
        yeti-vm/vm_handler.hxx
        yeti-vm/vm_interface.hxx
        yeti-vm/vm_memory.hxx
        yeti-vm/vm_syscall.hxx
        yeti-vm/vm_handlers_rv32i.hxx
        yeti-vm/vm_handlers_rv32m.hxx
)
set(LIB_SOURCES
        yeti-vm/vm_base_types.cxx
        yeti-vm/vm_utility.cxx
        yeti-vm/vm_opcode.cxx
        yeti-vm/vm_handler.cxx
        yeti-vm/vm_interface.cxx
        yeti-vm/vm_memory.cxx
        yeti-vm/vm_syscall.cxx
        yeti-vm/vm_handlers_rv32i.cxx
        yeti-vm/vm_handlers_rv32m.cxx
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
        yeti-vm/vm_basic.hxx
    PRIVATE
        yeti-vm/vm_basic.cxx
)
target_link_libraries(
    ${LIB_BASIC_VM}
    PUBLIC
        YetiVM::runtime
)
add_library(YetiVM::basic_vm ALIAS ${LIB_BASIC_VM})
