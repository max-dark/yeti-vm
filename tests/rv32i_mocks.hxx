#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <yeti-vm/vm_interface.hxx>

namespace tests::rv32i
{

struct MockVM: public vm::vm_interface
{
    MOCK_METHOD(void, halt, (), (override));

    MOCK_METHOD(void, jump_to, (offset_t value), (override));
    MOCK_METHOD(void, jump_if, (bool condition, offset_t value), (override));
    MOCK_METHOD(void, jump_if_abs, (bool condition, address_t value), (override));

    MOCK_METHOD(void, syscall, (), (override));
    MOCK_METHOD(void, debug, (), (override));
    MOCK_METHOD(void, control, (), (override));
    MOCK_METHOD(void, barrier, (), (override));

    MOCK_METHOD(void, read_memory, (address_t from, uint8_t size, vm::register_t& value), (override));
    MOCK_METHOD(void, write_memory, (address_t from, uint8_t size, vm::register_t value), (override));

    MOCK_METHOD(void, set_register, (vm::register_no r, vm::register_t value), (override));
    MOCK_METHOD(vm::register_t, get_register, (vm::register_no r), (const, override));
    MOCK_METHOD(vm::register_t, get_pc, (), (const, override));
};

} // namespace tests::rv32i
