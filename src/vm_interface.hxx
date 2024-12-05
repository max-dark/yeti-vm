#pragma once

#include <vm_base_types.hxx>

namespace vm
{

struct vm_interface
{
    using address_t = std::uint32_t;
    using offset_t = std::int32_t;

    virtual void jump_abs(address_t dest) = 0;

    virtual void jump_to(offset_t value) = 0;

    virtual void jump_if(bool condition, offset_t value) = 0;

    virtual void jump_if_abs(bool condition, address_t value) = 0;

    virtual void syscall() = 0;

    virtual void debug() = 0;

    virtual void control() = 0;

    virtual void barrier() = 0;

    virtual void read_memory(address_t from, uint8_t size, register_t& value) = 0;

    virtual void write_memory(address_t from, uint8_t size, register_t value) = 0;

    virtual void set_register(register_no r, register_t value) = 0;

    [[nodiscard]]
    virtual register_t get_register(register_no r) const = 0;

    [[nodiscard]]
    virtual register_t get_pc() const = 0;

    virtual ~vm_interface();
};

}// namespace vm
