cmake_minimum_required(VERSION 3.25)

project(YetiTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

find_package(GTest REQUIRED)
include(GoogleTest)

function(add_gtest)
    set(_options MOCK MAIN)
    set(_keys NAME COMMAND)
    set(_lists SOURCES LIBRARIES)
    cmake_parse_arguments(var "${_options}" "${_keys}" "${_lists}" ${ARGN})

    list(APPEND _libs YetiVM::runtime)
    list(APPEND _libs GTest::gtest)
    set(_main GTest::gtest)

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
    gtest_discover_tests(${var_COMMAND})
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

add_gtest(
        NAME "RV32I handlers"
        COMMAND rv32i_handlers
        MOCK MAIN # use GMock / own main function
        SOURCES
        rv32i_mocks.cxx rv32i_mocks.hxx
        rv32i_handlers.cxx
        rv32i_handlers_main.cxx
)
