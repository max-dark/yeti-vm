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
        vm_stdlib.hxx
)

set(APP_NAME risc-vm)

set(APP_SOURCES
    main.cxx
)

set(ELF_LIB elf-loader)
set(ELF_HEADERS
    elf_loader.hxx
)
set(ELF_SOURCES
    elf_loader.cxx
)
add_library(${ELF_LIB} STATIC)
target_sources(
    ${ELF_LIB}
    PRIVATE
        ${ELF_SOURCES}
    PUBLIC
        ${ELF_HEADERS}
)
target_link_libraries(
    ${ELF_LIB}
    PUBLIC
        shared
)

set(ELF_PARSER_APP elf-parser)
add_executable(${ELF_PARSER_APP})
target_sources(
    ${ELF_PARSER_APP}
    PRIVATE
        elf_parser.cxx
)
target_link_libraries(
    ${ELF_PARSER_APP}
    PRIVATE
        ${ELF_LIB}
)

set(LIB_NAME rvm)
set(LIB_HEADERS
    vm_base_types.hxx
    vm_utility.hxx
    vm_opcode.hxx
    vm_handler.hxx
    vm_interface.hxx
    vm_basic.hxx
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
    vm_basic.cxx
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
