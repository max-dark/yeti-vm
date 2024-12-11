cmake_minimum_required(VERSION 3.20)

project(YetiTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

add_executable(test_hex_checksum)
target_sources(
    test_hex_checksum
    PRIVATE
        hex_checksum.cxx
)

target_link_libraries(
    test_hex_checksum
    PRIVATE
        rvm
)

add_test(NAME "HEX checksum" COMMAND test_hex_checksum)

add_executable(test_hex_parser)
target_sources(
    test_hex_parser
    PRIVATE
        hex_parser.cxx
)

target_link_libraries(
    test_hex_parser
    PRIVATE
        rvm
)

add_test(NAME "HEX parser" COMMAND view_hex_file)

add_executable(view_hex_file)
target_sources(
    view_hex_file
    PRIVATE
        view_hex_file.cxx
)

target_link_libraries(
    view_hex_file
    PRIVATE
        rvm
)