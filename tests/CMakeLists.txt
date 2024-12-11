cmake_minimum_required(VERSION 3.20)

project(YetiTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

add_executable(mmu_tests)
target_sources(
    mmu_tests
    PRIVATE
        mmu_tests.cxx
)
target_link_libraries(
    mmu_tests
    PRIVATE
        rvm
)
add_test(NAME "MMU tests" COMMAND mmu_tests)
