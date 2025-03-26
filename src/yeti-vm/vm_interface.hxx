#pragma once

#include "vm_base_types.hxx"

namespace vm
{

/// Virtual machine interface
struct MachineInterface
{
    /// address in memory
    using address_t = std::uint32_t;
    /// address offset
    using offset_t = std::int32_t;

    /// stop VM
    virtual void halt() = 0;

    /// jump to absolute address
    virtual void jump_abs(address_t dest) = 0;

    /// jump to relative address
    virtual void jump_to(offset_t value) = 0;

    /// conditional jump by offset
    virtual void jump_if(bool condition, offset_t value) = 0;

    /// conditional jump
    virtual void jump_if_abs(bool condition, address_t value) = 0;

    /// system call
    virtual void syscall() = 0;

    /// debug break
    virtual void debug() = 0;

    /// CSR operations
    [[deprecated]]
    virtual void control() = 0;

    /// get CSR value
    virtual void control_get(address_t csr_id, register_t& value) = 0;

    /// set CSR value
    virtual void control_set(address_t csr_id, register_t value) = 0;

    /// memory barriers
    virtual void barrier() = 0;

    /// read(load) value from memory
    virtual void read_memory(address_t from, uint8_t size, register_t& value) = 0;

    /// write(store) value into memory
    virtual void write_memory(address_t from, uint8_t size, register_t value) = 0;

    /// set register value
    virtual void set_register(register_no r, register_t value) = 0;

    /// get register value
    [[nodiscard]]
    virtual register_t get_register(register_no r) const = 0;

    /// get PC register value
    [[nodiscard]]
    virtual register_t get_pc() const = 0;

    virtual ~MachineInterface();
};

}// namespace vm
