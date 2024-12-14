cmake_minimum_required(VERSION 3.20)

project(YetiExamples)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

include(riscv_toolchain)

find_riscv_toolchain()

riscv_add_executable(rv32i BIN HEX
        LINK_SCRIPT basic_vm.ld
        SOURCES rv32i.S
        startup.S
)

riscv_add_executable(rv32m BIN HEX
        LINK_SCRIPT basic_vm.ld
        SOURCES rv32m.S
        startup.S
)

riscv_add_executable(sys_calls BIN HEX
        LINK_SCRIPT basic_vm.ld
        SOURCES sys_calls_asm.S sys_calls_main.c
        startup.c
)

riscv_add_library(
        noname
        SOURCES lib_test.c
)
