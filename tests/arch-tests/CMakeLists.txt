cmake_minimum_required(VERSION 3.25)

project(YetiArchTests)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(riscv_toolchain)

find_riscv_toolchain()

add_executable(yeti_runner)
target_sources(yeti_runner
    PRIVATE
        yeti_runner.cxx
)
target_link_libraries(
    yeti_runner
    PRIVATE
        YetiVM::basic_vm
)

include(FetchContent)

# https://github.com/riscv-non-isa/riscv-arch-test/releases/tag/3.10.0
FetchContent_Populate(
        riscv-arch-test
        GIT_REPOSITORY https://github.com/riscv-non-isa/riscv-arch-test.git
        GIT_TAG fc32e41d49480fd99ba0a192dfff9c3319b44873
#        SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/riscv-arch-test
)

FetchContent_GetProperties(riscv-arch-test
    SOURCE_DIR ARCH_TEST_SOURCE
    BINARY_DIR ARCH_TEST_BINARY
    POPULATED  ARCH_TEST_POPULATED
)

message("riscv-arch-test: OK : ${ARCH_TEST_POPULATED}")
message("riscv-arch-test: SRC: ${ARCH_TEST_SOURCE}")
message("riscv-arch-test: BIN: ${ARCH_TEST_BINARY}")
