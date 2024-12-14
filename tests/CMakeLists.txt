cmake_minimum_required(VERSION 3.25)

project(YetiTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

option(YETI_ENABLE_TESTS "Build tests" ON)

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

if(YETI_ENABLE_TESTS)

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

endif()