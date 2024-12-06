#include "vm_basic.hxx"

namespace vm
{

void basic_vm::halt()
{
    running = false;
}

void basic_vm::jump_abs(basic_vm::address_t dest)
{
    bool valid = dest % sizeof(opcode::opcode_t);
    valid = valid && (dest < code.size());
    if (!valid)
    {
        return halt();
    }
    set_pc(dest);
}

void basic_vm::jump_to(basic_vm::offset_t value)
{
    auto dest = get_pc() + value;
    jump_abs(dest);
}

void basic_vm::jump_if(bool condition, basic_vm::offset_t value)
{
    if (condition) jump_to(value);
    else inc_pc();
}

void basic_vm::jump_if_abs(bool condition, basic_vm::address_t value)
{
    if (condition) jump_abs(value);
    else inc_pc();
}

void basic_vm::syscall()
{
    inc_pc();
}

void basic_vm::debug()
{
    inc_pc();
}

void basic_vm::control()
{
    inc_pc();
}

void basic_vm::barrier()
{
    inc_pc();
}

void basic_vm::read_memory(basic_vm::address_t from, uint8_t size, register_t &value)
{
    auto ptr = get_ptr(from);
    if (!ptr)
    {
        return halt();
    }
    if (size > sizeof(value))
    {
        return halt();
    }
    std::memcpy(&value, ptr, size);
    inc_pc();
}

void basic_vm::write_memory(basic_vm::address_t from, uint8_t size, register_t value)
{
    auto ptr = get_ptr(from);
    if (!ptr)
    {
        return halt();
    }
    if (size > sizeof(value))
    {
        return halt();
    }
    std::memcpy(ptr, &value, size);
    inc_pc();
}

void basic_vm::set_register(register_no r, register_t value)
{
    if (r > 0 && r <= register_count)
    {
        registers[r] = value;
    }
}

register_t basic_vm::get_register(register_no r) const
{
    if (r > 0 && r <= register_count)
    {
        return registers[r];
    }
    return 0;
}

register_t basic_vm::get_pc() const
{
    return get_register(RegAlias::pc);
}

void basic_vm::set_pc(register_t value)
{
    if (value + sizeof(opcode::opcode_t) >= code.size())
    {
        return halt();
    }
    set_register(RegAlias::pc, value);
}

void basic_vm::inc_pc()
{
    set_pc(get_pc() + sizeof(opcode::opcode_t));
}

const void *basic_vm::get_ptr(basic_vm::address_t address) const
{
    if (address >= data_base)
    {
        address -= data_base;
        if (address < data.size())
        {
            return data.data() + address;
        }
    }
    if (address >= code_base)
    {
        address -= code_base;
        if (address < code.size())
        {
            return code.data() + address;
        }
    }
    return nullptr;
}

void *basic_vm::get_ptr(basic_vm::address_t address)
{
    if (address >= data_base)
    {
        address -= data_base;
        if (address < data.size())
        {
            return data.data() + address;
        }
    }
    if (address >= code_base)
    {
        address -= code_base;
        if (address < code.size())
        {
            return code.data() + address;
        }
    }
    return nullptr;
}

void basic_vm::set_ro_size(size_t size)
{
    code.resize(size);
}

void basic_vm::set_rw_size(size_t size)
{
    data.resize(size);
}

void basic_vm::run_step()
{
    auto* current = get_current();
    auto handler = find_handler(current);
    if (!handler)
    {
        return halt();
    }
    handler->exec(this, current);
}

void basic_vm::run()
{
    while (is_running())
    {
        run_step();
    }
}

void basic_vm::start()
{
    std::fill(registers.begin(), registers.end(), 0);
    std::fill(data.begin(), data.end(), 0);
    running = true;
}

bool basic_vm::is_running() const
{
    return running;
}

void basic_vm::init_memory()
{
    init_memory(def_code_size, def_data_size);
}

void basic_vm::init_memory(size_t code_size, size_t data_size)
{
    set_ro_size(code_size);
    set_rw_size(data_size);
}

bool basic_vm::set_program(const program_code_t &bin)
{
    if (!is_initialized()) return false;
    if (bin.size() > code.size()) return false;

    std::copy(bin.begin(), bin.end(), code.begin());

    return true;
}

bool basic_vm::is_initialized() const
{
    return !(code.empty() || data.empty());
}

const opcode::OpcodeBase *basic_vm::get_current() const
{
    const auto * ptr = code.data() + get_pc();
    return reinterpret_cast<const opcode::OpcodeBase*>(ptr);
}

void basic_vm::set_ro_base(basic_vm::address_t base)
{
    code_base = base;
}

void basic_vm::set_rw_base(basic_vm::address_t base)
{
    data_base = base;
}

} // namespace vm
