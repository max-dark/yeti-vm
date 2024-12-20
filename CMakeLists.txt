cmake_minimum_required(VERSION 3.25)

include(cmake/YetiVersion.cmake)

project(Yeti LANGUAGES CXX
        VERSION ${YetiVersion}
        DESCRIPTION "${YetiProject}"
)

option(YETI_ENABLE_TESTS "Build tests" ON)
option(YETI_ENABLE_ARCH_TESTS "Build riscv-arch-tests tests" ON)
option(YETI_ENABLE_TOOLS "Build tools" ON)
option(YETI_ENABLE_EXAMPLES "Build examples" ON)
option(YETI_ENABLE_INSTALL "Add install targets for libraries" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if(YETI_ENABLE_EXAMPLES)
    add_subdirectory(examples)
endif() # YETI_ENABLE_EXAMPLES

add_subdirectory(src)

if (YETI_ENABLE_TOOLS)
    add_subdirectory(tools)
endif() # YETI_ENABLE_TOOLS

if(YETI_ENABLE_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif() # YETI_ENABLE_TESTS

if (YETI_ENABLE_ARCH_TESTS)
    enable_testing()
    add_subdirectory(tests/arch-tests)
endif ()
