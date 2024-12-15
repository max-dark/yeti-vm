#include "vm_basic.hxx"

#include "vm_handlers_rv32i.hxx"
#include "vm_handlers_rv32m.hxx"

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
        throw data_access_error{std::format("load: address {:08x} out of range", from)};;
    }
    if (size > sizeof(value)) [[unlikely]]
    {
        throw data_access_error{"load: size out of range"};
    }
    bool ok = ptr->load(from, &value, size);
    if (!ok) [[unlikely]]
    {
        throw data_access_error{"load: read error"};
    }
}

void basic_vm::write_memory(basic_vm::address_t from, uint8_t size, register_t value)
{
    if ((size == 0) || (size > sizeof(value))) [[unlikely]]
    {
        throw data_access_error{"store: size out of range"};
    }
    auto ptr = get_ptr_rw(from, size);
    if (!ptr) [[unlikely]]
    {
        throw data_access_error{std::format("store: address {:08x} out of range", from)};
    }
    bool ok = ptr->store(from, &value, size);
    if (!ok) [[unlikely]]
    {
        throw data_access_error{"store: write error"};
    }
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
    if (!mmu.find_block(value, sizeof(opcode::opcode_t))) [[unlikely]]
    {
        throw code_access_error{std::format("destination address {:08x} outside code region", value)};
    }
    registers[RegAlias::pc] = value;
}

void basic_vm::inc_pc()
{
    set_pc(get_pc() + sizeof(opcode::opcode_t));
}

const memory_block *basic_vm::get_ptr_ro(address_t address, uint8_t size) const
{
    if (address % size)
    {
        throw data_access_error{std::format("address {:08x} is not aligned", address)};
    }
    if (auto ptr = mmu.find_block(address, size))
        return ptr;
    return nullptr;
}

memory_block *basic_vm::get_ptr_rw(vm::vm_interface::address_t address, uint8_t size)
{
    if (address % size)
    {
        throw data_access_error{std::format("address {:08x} is not aligned", address)};
    }
    if (auto ptr = mmu.find_block(address, size))
        return ptr;
    return nullptr;
}

bool basic_vm::set_ro_size(size_t size)
{
    if (is_flag_set(HAVE_CODE_BLOCK)) return false;
    ro_size = size;
    return true;
}

bool basic_vm::set_rw_size(size_t size)
{
    if (is_flag_set(HAVE_DATA_BLOCK)) return false;
    rw_size = size;
    return true;
}

void basic_vm::run_step()
{
    auto* current = get_current();
    if (!current) [[unlikely]]
    {
        throw unknown_instruction{std::format("unable fetch instruction from {:08x}", get_pc())};
    }
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
                << std::setw(20) << std::setfill(' ') << std::left << handler->get_args(current)
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
    set_pc(initial_pc);
    running = is_initialized();
}

bool basic_vm::is_running() const
{
    return running;
}

bool basic_vm::init_memory()
{
    return init_memory(def_code_size, def_data_size);
}

bool basic_vm::init_memory(size_t code_size, size_t data_size)
{
    bool ro_ok = set_ro_size(code_size);
    bool rw_ok = set_rw_size(data_size);

    bool mem_ok = ro_ok && rw_ok;
    mem_ok &= add_code_block(code_base, ro_size);
    mem_ok &= add_data_block(data_base, rw_size);

    return mem_ok;
}

bool basic_vm::set_program(const program_code_t &bin, address_t pc_value)
{
    if (!have_code_block()) return false;
    if (!init_pc(pc_value)) return false;
    auto code = mmu.find_block(code_base, bin.size());
    if (!code) return false;
    return code->store(code_base, bin.data(), bin.size());
}

bool basic_vm::set_program(const hex_file &hex)
{
    if (!have_code_block()) return false; // no memory for code
    if (is_flag_set(PC_INITIALIZED)) return false; // program loaded

    address_t offset = 0;
    address_t pc_value = 0;

    for (const hex_record& record: hex)
    {
        if (!record.is_valid()) [[unlikely]] return false;

        switch (record.get_type())
        {
        case hex_record::HEX_DATA: // load data to memory
        {
            auto data_ptr = record.data.data();
            auto data_sz = record.data.size();
            address_t address = offset + record.offset;
            auto mem_block = mmu.find_block(address, data_sz);

            bool ok = mem_block != nullptr;
            ok = ok && data_ptr != nullptr;
            ok = ok && mem_block->store(address, data_ptr, data_sz);
            if (!ok) [[unlikely]] return false;
            break;
        }
        case hex_record::HEX_EOF:
        {
            return init_pc(pc_value); // EOF record: load finished
        }
        case hex_record::HEX_SEGMENT_START:
        case hex_record::HEX_LINEAR_START:
        {
            pc_value = record.get_start(); // use as entry point
            break;
        }
        case hex_record::HEX_SEGMENT_EXTEND:
        case hex_record::HEX_LINEAR_EXTEND:
        {
            offset = record.get_extend(); // offset for next data records
            break;
        }
        case hex_record::HEX_UNKNOWN:
            return false; // can't load
        }
    }

    return false; // EOF record is required
}

bool basic_vm::is_initialized() const
{
    return initFlags == ALL_FLAGS_MASK;
}

const opcode::OpcodeBase *basic_vm::get_current() const
{
    const auto * ptr = get_ptr_ro(get_pc(), sizeof(opcode::opcode_t));
    return ptr->get_ro_ptr<opcode::OpcodeBase>(get_pc());
}

bool basic_vm::set_ro_base(address_t base)
{
    if (have_code_block()) return false;
    code_base = base;
    return true;
}

bool basic_vm::set_rw_base(address_t base)
{
    if (have_data_block()) return false;
    data_base = base;
    return true;
}

bool basic_vm::init_isa()
{
    bool rv32i_ok = rv32i::register_rv32i_set(&opcodes);
    bool rv32m_ok = rv32m::register_rv32m_set(&opcodes);

    bool isa_ok = rv32i_ok && rv32m_ok;

    if (isa_ok)
    {
        set_flag(ISA_INITIALIZED);
    }

    return isa_ok;
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

bool basic_vm::add_memory(vm_interface::address_t address, size_t size)
{
    return mmu.add_block<vm::generic_memory>(address, size);
}

bool basic_vm::add_memory(memory_block::ptr ptr)
{
    return mmu.add_block(std::move(ptr));
}

bool basic_vm::add_code_block(vm_interface::address_t address, size_t size)
{
    if (have_code_block()) return false;
    if (!add_memory(address, size)) return false;
    set_flag(HAVE_CODE_BLOCK);
    return true;
}

bool basic_vm::add_data_block(vm_interface::address_t address, size_t size)
{
    if (have_data_block()) return false;
    if (!add_memory(address, size)) return false;
    set_flag(HAVE_DATA_BLOCK);
    return true;
}

bool basic_vm::is_flag_set(basic_vm::InitFlag flag) const
{
    return initFlags & flag;
}

void basic_vm::set_flag(basic_vm::InitFlag flag)
{
    initFlags |= flag;
}

void basic_vm::clear_flag(basic_vm::InitFlag flag)
{
    initFlags &= ~flag;
}

bool basic_vm::init_pc(vm_interface::address_t address)
{
    if (is_flag_set(PC_INITIALIZED)) return false;
    initial_pc = address;
    set_flag(PC_INITIALIZED);
    return true;
}

bool basic_vm::have_code_block() const
{
    return is_flag_set(HAVE_CODE_BLOCK);
}

bool basic_vm::have_data_block() const
{
    return is_flag_set(HAVE_DATA_BLOCK);
}

void basic_vm::dump_state(std::ostream &dump) const
{
    {
        auto code = get_current();
        if (code)
        {
            auto handler = opcodes.find_handler(code);
            dump << "current: " << vm::opcode::get_op_id(static_cast<vm::opcode::OpcodeType>(code->get_code())) << std::endl;
            dump << "\tcode: " << std::hex << code->code << std::endl;
            dump << "\thandler: " << (handler ? "found" : "not found" ) << std::endl;

            dump
                    << "\trd  = " << std::setw(4) << vm::get_register_alias(code->get_rd())
                    << std::endl
                    << "\trs1 = " << std::setw(4) << vm::get_register_alias(code->get_rs1())
                    << std::endl
                    << "\trs2 = " << std::setw(4) << vm::get_register_alias(code->get_rs2())
                    << std::endl
                    << std::hex
                    << "\ti = " << std::setw(4) << code->decode_i_u() << std::endl
                    << "\tb = " << std::setw(4) << code->decode_b_u() << std::endl
                    << "\tj = " << std::setw(4) << code->decode_j_u() << std::endl
                    << "\ts = " << std::setw(4) << code->decode_s_u() << std::endl
                    << "\tu = " << std::setw(4) << code->decode_u_u() << std::endl
                ;
            if (handler)
            {
                dump << "\tdecoded: " << handler->get_mnemonic() << "\t" << handler->get_args(code) << std::endl;
            }
        }
        else
        {
            dump << "unable fetch instruction" << std::endl;
        }
    }
    dump << "REG dump:" << std::endl;
    dump << "PC: " << std::setw(10) << std::hex << get_pc() << std::endl;

    for (vm::register_no i = 0; i < vm::register_count; ++i)
    {
        dump
            << std::dec << std::setw(2) << std::uint32_t(i)
            << std::setw(4) << vm::get_register_alias(i)
            << std::setw(10) << std::hex << get_register(i) << std::endl;
    }

}

} // namespace vm
