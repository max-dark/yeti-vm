cmake_minimum_required(VERSION 3.25)

include(cmake/YetiVersion.cmake)

project(Yeti LANGUAGES CXX
        VERSION ${YetiVersion}
        DESCRIPTION "${YetiProject}"
)

include(ExternalProject)

option(YETI_ENABLE_TESTS "Build tests" ON)
option(YETI_ENABLE_ARCH_TESTS "Build riscv-arch-tests tests" ON)
option(YETI_ENABLE_TOOLS "Build tools" ON)
option(YETI_ENABLE_EXAMPLES "Build examples" ON)
option(YETI_ENABLE_INSTALL "Add install targets for libraries" ON)

set(DOWNLOAD_BASE_DIR "${CMAKE_CURRENT_LIST_DIR}/vendor/" CACHE STRING "Base dir for downloads")


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

    ExternalProject_Add(riscv_arch_test
            GIT_REPOSITORY https://github.com/riscv-non-isa/riscv-arch-test.git
            GIT_TAG fc32e41d49480fd99ba0a192dfff9c3319b44873 # 3.10.0
            GIT_PROGRESS ON
            SOURCE_DIR "${DOWNLOAD_BASE_DIR}/riscv-arch-test"
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ""
            INSTALL_COMMAND ""
            BUILD_IN_SOURCE ON
    )

    add_subdirectory(tests/arch-tests)
endif ()
