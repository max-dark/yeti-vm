#include "vm_basic.hxx"

#include <vm_handlers_rv32i.hxx>
#include <vm_handlers_rv32m.hxx>

#include <iostream>
#include <format>

namespace vm
{

void basic_vm::halt()
{
    running = false;
}

void basic_vm::jump_abs(basic_vm::address_t dest)
{
    if (dest % sizeof(opcode::opcode_t)) [[unlikely]]
    {
        throw code_access_error{
            std::format("destination address({:08x}) should be aligned by instruction size", dest)
        };
    };
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
    auto syscall_id = syscalls.get_syscall_id(this);
    auto handler = syscalls.find_handler(syscall_id);
    if (handler == nullptr) [[unlikely]]
    {
        throw unknown_syscall{
                std::format("unknown syscall #{:08x}", syscall_id)
        };
    }
    handler->exec(this);
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
    auto ptr = get_ptr_ro(from, size);
    if (!ptr) [[unlikely]]
    {
        throw data_access_error{std::format("address {:08x} out of range", from)};;
    }
    if (size > sizeof(value)) [[unlikely]]
    {
        throw data_access_error{"size out of range"};
    }
    std::memcpy(&value, ptr, size);
}

void basic_vm::write_memory(basic_vm::address_t from, uint8_t size, register_t value)
{
    if ((size == 0) || (size > sizeof(value))) [[unlikely]]
    {
        throw data_access_error{"size out of range"};
    }
    auto ptr = get_ptr_rw(from, size);
    if (!ptr) [[unlikely]]
    {
        throw data_access_error{std::format("address {:08x} out of range", from)};
    }
    std::memcpy(ptr, &value, size);
}

void basic_vm::set_register(register_no r, register_t value)
{
    if (r >= register_count) [[unlikely]]
    {
        throw data_access_error{std::format("register ID({}) out of range", r)};
    }
    if (r > 0)
    {
        registers[r] = value;
    }
}

register_t basic_vm::get_register(register_no r) const
{
    if (r >= register_count) [[unlikely]]
    {
        throw data_access_error{std::format("register ID({}) out of range", r)};
    }
    if (r > 0)
    {
        return registers[r];
    }
    return 0;
}

register_t basic_vm::get_pc() const
{
    return registers[RegAlias::pc];
}

void basic_vm::set_pc(register_t value)
{
    if (value + sizeof(opcode::opcode_t) >= code.size()) [[unlikely]]
    {
        throw code_access_error{std::format("destination address {:08x} outside code region", value)};
    }
    registers[RegAlias::pc] = value;
}

void basic_vm::inc_pc()
{
    set_pc(get_pc() + sizeof(opcode::opcode_t));
}

const void *basic_vm::get_ptr_ro(address_t address, uint8_t size) const
{
    auto mem_check = [address, size](auto& block, address_t base) -> const void *
    {
        auto block_size = block.size() * sizeof(block.at(0));
        if (address >= base)
        {
            address_t offset = address - base;
            if (offset < block_size)
            {
                if ((offset + size) > block_size)
                {
                    throw data_access_error{"size out of range"};
                }
                return block.data() + offset;
            }
        }
        return nullptr;
    };

    if (address % size)
    {
        throw data_access_error{std::format("address {:08x} is not aligned", address)};
    }
    if (auto ptr = mem_check(data, data_base))
        return ptr;
    if (auto ptr = mem_check(code, code_base))
        return ptr;
    return nullptr;
}

void *basic_vm::get_ptr_rw(vm::vm_interface::address_t address, uint8_t size)
{
    auto mem_check = [address, size](auto& block, address_t base) -> void *
    {
        auto block_size = block.size() * sizeof(block.at(0));
        if (address >= base)
        {
            address_t offset = address - base;
            if (offset < block_size)
            {
                if ((offset + size) > block_size)
                {
                    throw data_access_error{"size out of range"};
                }
                return block.data() + offset;
            }
        }
        return nullptr;
    };

    if (address % size)
    {
        throw data_access_error{std::format("address {:08x} is not aligned", address)};
    }
    if (auto ptr = mem_check(data, data_base))
        return ptr;
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
    auto handler = opcodes.find_handler(current);
    if (!handler) [[unlikely]]
    {
        throw unknown_instruction{std::format("unable find handler for {:08x}", current->code)};
    }
    if (is_debugging_enabled()) [[unlikely]]
    {
        std::cout
                << std::setw( 8) << std::setfill('0') << std::right << get_pc() << ' '
                << std::setw(10) << std::setfill(' ') << std::left << handler->get_mnemonic()
                << std::setw(10) << std::setfill(' ') << std::left << handler->get_args(current)
                << std::setw(10) << std::right << std::hex << get_register(current->get_rd())
                << std::setw(10) << std::right << std::hex << get_register(current->get_rs1())
                << std::setw(10) << std::right << std::hex << get_register(current->get_rs2())
        ;
    }
    handler->exec(this, current);
    if (is_debugging_enabled()) [[unlikely]]
    {
        std::cout
                << std::setw(10) << std::right << std::hex << get_register(current->get_rd())
                << std::setw(10) << std::right << std::hex << get_register(current->get_rs1())
                << std::setw(10) << std::right << std::hex << get_register(current->get_rs2())
                << std::endl;
    }

    if (!handler->skip()) [[likely]]
    {
        inc_pc();
    }
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

    set_register(vm::RegAlias::gp, data_base + data.size());
    set_register(vm::RegAlias::sp, data_base + data.size());
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

void basic_vm::init_isa()
{
    rv32i::register_rv32i_set(&opcodes);
    rv32m::register_rv32m_set(&opcodes);
}

syscall_registry &basic_vm::get_syscalls()
{
    return syscalls;
}

bool basic_vm::is_debugging_enabled() const
{
    return debugging;
}

void basic_vm::enable_debugging(bool enable)
{
    debugging = enable;
}

} // namespace vm
