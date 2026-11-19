#pragma once

#include "vm_base_types.hxx"
#include "vm_interface.hxx"

namespace vm
{
struct hart_base : public HartInterface
{
    void halt() override;

    void jump_abs(address_t dest) override;

    void jump_to(offset_t value) override;

    void jump_if(bool condition, offset_t value) override;

    void jump_if_abs(bool condition, address_t value) override;

    void syscall() override;

    void debug() override;

    void control_get(address_t csr_id, register_t &value) override;

    void control_set(address_t csr_id, register_t value) override;

    void barrier() override;

    void read_memory(address_t from, uint8_t size, register_t &value) override;

    void write_memory(address_t from, uint8_t size, register_t value) override;

    void set_register(register_no r, register_t value) override;

    [[nodiscard]] register_t get_register(register_no r) const override;

    [[nodiscard]] register_t get_pc() const override;

    /// set PC register value
    virtual void set_pc(register_t value) = 0;
};
} // namespace vm
