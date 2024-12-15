cmake_minimum_required(VERSION 3.25)

project(YetiVM LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(YetiUtility)

add_library(yeti_vm_shared INTERFACE)
add_include_dir(yeti_vm_shared INTERFACE)

add_header_files(
    yeti_vm_shared
    INTERFACE HEADERS
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
)
add_header_files(
    ${LIB_NAME}
    PUBLIC HEADERS ${LIB_HEADERS}
)
target_link_libraries(
    ${LIB_NAME}
    PUBLIC
        YetiVM::shared
)
add_include_dir(${LIB_NAME} PUBLIC)
add_library(YetiVM::runtime ALIAS ${LIB_NAME})

set(LIB_BASIC_VM yeti_vm_basic)
add_library(${LIB_BASIC_VM} STATIC)
target_sources(
    ${LIB_BASIC_VM}
    PRIVATE
        yeti-vm/vm_basic.cxx
)
add_header_files(
    ${LIB_BASIC_VM}
    PUBLIC HEADERS yeti-vm/vm_basic.hxx
)
target_link_libraries(
    ${LIB_BASIC_VM}
    PUBLIC
        YetiVM::runtime
)
add_library(YetiVM::basic_vm ALIAS ${LIB_BASIC_VM})

include(GNUInstallDirs)

# standard paths are used, so specifying 'DESTINATION ...' is not necessary
install(TARGETS
        yeti_vm_shared
        yeti_vm_runtime
        yeti_vm_basic
        EXPORT YetiVMTargets
        LIBRARY COMPONENT Runtime
        RUNTIME COMPONENT Runtime
        ARCHIVE COMPONENT Libraries
#        PUBLIC_HEADER COMPONENT Headers # target_sources produce cmake error when add headers as PUBLIC files
        FILE_SET HEADERS COMPONENT Headers
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT YetiVMTargets
#        CONFIGURATIONS Debug Release
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/YetiVM
)
