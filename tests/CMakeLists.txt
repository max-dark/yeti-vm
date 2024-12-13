cmake_minimum_required(VERSION 3.25)

project(YetiTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

find_package(GTest REQUIRED)
include(GoogleTest)

function(add_gtest)
    set(_options MOCK MAIN NO_DISCOVER)
    set(_keys NAME COMMAND)
    set(_lists SOURCES LIBRARIES)
    cmake_parse_arguments(var "${_options}" "${_keys}" "${_lists}" ${ARGN})

    list(APPEND _libs YetiVM::runtime)
    list(APPEND _libs GTest::gtest)
    set(_main GTest::gtest_main)

    if (var_MOCK)
        list(APPEND _libs GTest::gmock)
        set(_main GTest::gmock_main)
    endif ()

    if (NOT var_MAIN)
        list(APPEND _libs "${_main}")
    endif ()

    add_executable(${var_COMMAND})
    target_sources(${var_COMMAND} PRIVATE ${var_SOURCES})

    target_link_libraries(${var_COMMAND} PRIVATE ${_libs} ${var_LIBRARIES})

    if (var_NO_DISCOVER)
        add_test(NAME "${var_NAME}" COMMAND ${var_COMMAND} "--gtest_filter=*" "--gtest_color=no")
    else ()
        gtest_discover_tests(${var_COMMAND})
    endif ()
endfunction()

# yeti_add_test(NAME <"title"> COMMAND <exe_name> SOURCES <sources> [LIBRARIES <libs>])
#
# NAME - test title, required
# COMMAND - executable name, required
# SOURCES - list of sources, required
# LIBRARIES - additional libraries, optional
function(yeti_add_test)
    set(_options)
    set(_keys NAME COMMAND)
    set(_lists SOURCES LIBRARIES)
    cmake_parse_arguments(var "${_options}" "${_keys}" "${_lists}" ${ARGN})

    add_executable(${var_COMMAND})
    target_sources(${var_COMMAND} PRIVATE ${var_SOURCES})

    target_link_libraries(${var_COMMAND} PRIVATE YetiVM::runtime ${var_LIBRARIES})
    add_test(NAME "${var_NAME}" COMMAND ${var_COMMAND})
endfunction()

yeti_add_test(
        NAME "HEX checksum"
        COMMAND test_hex_checksum
        SOURCES hex_checksum.cxx
)

yeti_add_test(
        NAME "HEX parser"
        COMMAND test_hex_parser
        SOURCES hex_parser.cxx
)

yeti_add_test(
        NAME "MMU tests"
        COMMAND mmu_tests
        SOURCES mmu_tests.cxx
)

add_gtest(
        NAME "RV32I tests"
        COMMAND rv32i_tests
        SOURCES
        rv32i_code.cxx
        rv32i_immediate.cxx
)

# This test is parametrised with big ranges of values
# with default `gtest_discover_tests` produces `add_test` for each variant(about 20k tests oO)
# It is too slow because CTest spawn new process for each test
# So. disable `gtest_discover_tests`
add_gtest(
        NO_DISCOVER
        NAME "RV32 Encoder/Decoder tests"
        COMMAND rv32_encode_decode
        SOURCES
        rv32_encoder.cxx
)

add_library(yeti_vm_mocks STATIC)
target_sources(
        yeti_vm_mocks
        PRIVATE
        rv32_vm_mocks.cxx
        PUBLIC
        rv32_vm_mocks.hxx
)
target_link_libraries(
        yeti_vm_mocks
        PUBLIC
        YetiVM::runtime
        GTest::gmock GTest::gtest
)

add_gtest(
        NAME "RV32I handlers"
        COMMAND rv32i_handlers
        MOCK MAIN # use GMock / own main function
        LIBRARIES
        yeti_vm_mocks
        SOURCES
        rv32i_handlers_main.cxx
        rv32i_handlers.cxx
        rv32i_handlers.hxx
        rv32i_handlers_impl.cxx
        rv32i_handlers_branch.cxx
        rv32i_handlers_load.cxx
        rv32i_handlers_store.cxx
        rv32i_handlers_integer_ri.cxx
        rv32i_handlers_integer_rr.cxx
        rv32i_handlers_misc_mem.cxx
        rv32i_handlers_system.cxx
)
