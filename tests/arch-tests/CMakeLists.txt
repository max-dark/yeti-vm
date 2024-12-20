cmake_minimum_required(VERSION 3.25)

project(YetiArchTests)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(riscv_toolchain)

find_riscv_toolchain()

add_executable(yeti-runner)
target_sources(yeti-runner
    PRIVATE
        yeti_runner.cxx
)
target_link_libraries(
    yeti-runner
    PRIVATE
        YetiVM::basic_vm
)

add_dependencies(yeti-runner riscv_arch_test)

ExternalProject_Get_Property(riscv_arch_test SOURCE_DIR)
set(ARCH_TEST_SUITE_DIR "${SOURCE_DIR}/riscv-test-suite")
set(ARCH_TEST_SUITE_ENV "${ARCH_TEST_SUITE_DIR}/env")
set(ARCH_TEST_SUITE_RV32 "${ARCH_TEST_SUITE_DIR}/rv32i_m")

list(APPEND ARCH_TEST_INCLUDE_DIRS
        "-I${ARCH_TEST_SUITE_ENV}"
        "-I${CMAKE_CURRENT_LIST_DIR}/config"
)
# list of ISA subsets
list(APPEND ARCH_TEST_SUBSETS I M privilege)
list(APPEND YETI_VM_ARCH_ARGS -march=rv32im -mabi=ilp32 -mcmodel=medany -nostdlib -static -nostartfiles)
# use POST_BUILD step to compile tests
block()
    set(_out_dir "${CMAKE_CURRENT_BINARY_DIR}")
    foreach (_subset IN LISTS ARCH_TEST_SUBSETS)
        set(_subset_dir "${ARCH_TEST_SUITE_RV32}/${_subset}/src/")
        file(GLOB _subset_tests RELATIVE "${_subset_dir}" "${_subset_dir}/*.S")
        message(DEBUG "Dir: ${_subset_dir} ... tests: ${_subset_tests}")

        foreach (_test_asm IN LISTS _subset_tests)
            get_filename_component(_test_name "${_test_asm}" NAME_WLE)
            set(_test_id "${_subset}/${_test_name}")
            message(DEBUG "Add test ${_test_id}")
            set(_input_file "${_subset_dir}/${_test_asm}")
            set(_elf_file "${_out_dir}/${_subset}_${_test_name}.elf")
            set(_hex_file "${_out_dir}/${_subset}_${_test_name}.hex")
            list(APPEND _build_args
                    "${YETI_VM_ARCH_ARGS}"
                    "${ARCH_TEST_INCLUDE_DIRS}"
                    -T "${CMAKE_CURRENT_LIST_DIR}/config/link.ld"
                    "${_input_file}"
                    -o "${_elf_file}"
            )
            add_custom_command(TARGET yeti-runner POST_BUILD
                    COMMENT "Build ${_test_id}"
                    COMMAND rv_tools::_gcc
                    ARGS "${_build_args}"
                    DEPENDS "${_input_file}"
                        "${CMAKE_CURRENT_LIST_DIR}/config/link.ld"
                        "${CMAKE_CURRENT_LIST_DIR}/config/model_test.h"
                    BYPRODUCTS "${_elf_file}"
                    COMMAND_EXPAND_LISTS
            )
            add_custom_command(TARGET yeti-runner POST_BUILD
                    COMMENT "Make hex file ${_test_id}"
                    COMMAND rv_tools::_objcopy
                    ARGS -O ihex "${_elf_file}" "${_hex_file}"
                    DEPENDS "${_elf_file}"
                    BYPRODUCTS "${_hex_file}"
            )
            unset(_input_file)
            unset(_elf_file)
            unset(_hex_file)
            unset(_build_args)
        endforeach ()
        unset(_subset_dir)
    endforeach ()
    unset(_subset)
    unset(_out_dir)
endblock()
