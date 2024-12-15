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

if (YETI_ENABLE_INSTALL)

# using *NIX-style dir structure
include(GNUInstallDirs)

# standard paths are used, so specifying 'DESTINATION ...' is not necessary
install(TARGETS
        yeti_vm_shared
        yeti_vm_runtime
        yeti_vm_basic
        EXPORT "${YetiVM_Package}Targets"
        LIBRARY COMPONENT Runtime
        RUNTIME COMPONENT Runtime
        ARCHIVE COMPONENT Development
#        PUBLIC_HEADER COMPONENT Headers # target_sources produce cmake error when add headers as PUBLIC files
        FILE_SET HEADERS COMPONENT Development
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

set(_package_prefix "${YetiVM_Package}")
# install data to INSTALL_PREFIX/share
set(_project_data_dir "${CMAKE_INSTALL_DATADIR}/${YetiVM_Package}-${YetiVersion}")
set(_cmake_install_dir "${_project_data_dir}/cmake")

install(EXPORT "${YetiVM_Package}Targets"
#        CONFIGURATIONS Debug Release
        DESTINATION ${_cmake_install_dir}
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
        "${CMAKE_CURRENT_LIST_DIR}/PackageConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/${_package_prefix}Config.cmake"
        INSTALL_DESTINATION ${_cmake_install_dir}
        NO_SET_AND_CHECK_MACRO
        NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/${_package_prefix}ConfigVersion.cmake"
        VERSION "${YetiVersion}"
        COMPATIBILITY ExactVersion # TODO: do not forget change after API stabilisation
)

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/${_package_prefix}Config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/${_package_prefix}ConfigVersion.cmake"
        COMPONENT Development
        DESTINATION ${_cmake_install_dir}
)

unset(_cmake_install_dir)
unset(_project_data_dir)
unset(_package_prefix)

endif() # YETI_ENABLE_INSTALL
