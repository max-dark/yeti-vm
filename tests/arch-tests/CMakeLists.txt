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