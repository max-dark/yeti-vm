#include "vm_hart_base.hxx"

void vm::hart_base::halt()
{
}

void vm::hart_base::jump_abs(address_t dest)
{
}

void vm::hart_base::jump_to(offset_t value)
{
}

void vm::hart_base::jump_if(bool condition, offset_t value)
{
}

void vm::hart_base::jump_if_abs(bool condition, address_t value)
{
}

void vm::hart_base::syscall()
{
}

void vm::hart_base::debug()
{
}

void vm::hart_base::control_get(address_t csr_id, register_t &value)
{
}

void vm::hart_base::control_set(address_t csr_id, register_t value)
{
}

void vm::hart_base::barrier()
{
}

void vm::hart_base::read_memory(address_t from, uint8_t size, register_t &value)
{
}

void vm::hart_base::write_memory(address_t from, uint8_t size, register_t value)
{
}

void vm::hart_base::set_register(register_no r, register_t value)
{
}

vm::register_t vm::hart_base::get_register(register_no r) const
{
    return 0;
}

vm::register_t vm::hart_base::get_pc() const
{
    return 0;
}
