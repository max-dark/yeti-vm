#include "stdlib.hxx"

#include <vm_opcode.hxx>
#include <vm_handler.hxx>
#include <vm_basic.hxx>
#include <vm_handlers_rv32i.hxx>
#include <vm_handlers_rv32m.hxx>
#include <vm_base_types.hxx>
#include <vm_utility.hxx>

#include <iostream>
#include <format>

namespace fs = std::filesystem;

using program = vm::program_code_t;

std::optional<program> load_program(const fs::path& programFile)
{
    program data;

    if (fs::is_regular_file(programFile))
    {
        auto sz = static_cast<std::streamsize>(fs::file_size(programFile));
        data.resize(sz);
        std::ifstream bin{programFile, std::ios::binary};

        if (bin.is_open())
        {
            bin.read(reinterpret_cast<char *>(data.data()), sz);
            if (bin.good() || bin.eof())
                return data;
        }
    }

    return std::nullopt;
}

void disasm(const fs::path &program_file);
void disasm(const program &code);

void run_vm(const fs::path &program_file, bool debug);
void run_vm(const program &code, bool debug);

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cout << "Usage: exe <v|d> <path/to/file.bin>" << std::endl;
        return 0;
    }

    fs::path program_file = argv[2];

    switch(*argv[1])
    {
    case 'd':
        disasm(program_file);
        break;
    case 'v':
        run_vm(program_file, false);
        break;
    case 'V':
        run_vm(program_file, true);
        break;
    default:
        std::cout << "Unknown option: " << argv[1] << std::endl;
        break;
    }

    return 0;
}

void run_vm(const program &code, bool debug)
{
    vm::basic_vm machine;

    machine.enable_debugging(debug);
    machine.set_rw_base(0);
    {
        using vm::RegAlias;
        using call = vm::syscall_functor;
        auto& sys = machine.get_syscalls();
        sys.register_handler(call::create(1024, "open", [](vm::vm_interface* m){
            auto name_ptr = m->get_register(RegAlias::a0);
            auto flags = m->get_register(RegAlias::a1);
            std::cout << "open " << vm::to_signed(name_ptr) << " " << flags << std::endl;
            m->set_register(RegAlias::a0, 0);
        }));
        sys.register_handler(call::create(63, "read", [](vm::vm_interface* m){
            auto file_id = m->get_register(RegAlias::a0);
            auto buff_ptr = m->get_register(RegAlias::a1);
            auto buff_sz = m->get_register(RegAlias::a2);
            std::cout << "read " << file_id << " " << vm::to_signed(buff_ptr) << " " << buff_sz << std::endl;
            m->set_register(RegAlias::a0, 0);
        }));
        sys.register_handler(call::create(57, "close", [](vm::vm_interface* m){
            auto file_id = m->get_register(RegAlias::a0);
            std::cout << "close " << file_id << std::endl;
            m->set_register(RegAlias::a0, 0);
        }));
        sys.register_handler(call::create(11, "put_char", [](vm::vm_interface* m){
            auto value = m->get_register(RegAlias::a0);
            auto data = reinterpret_cast<const char*>(&value);
            std::cout << *data;
            m->set_register(RegAlias::a0, 0);
        }));
        sys.register_handler(call::create(1, "put_int", [](vm::vm_interface* m){
            auto value = m->get_register(RegAlias::a0);
            std::cout << vm::to_signed(value);
            m->set_register(RegAlias::a0, 0);
        }));
        sys.register_handler(call::create(10, "exit", [](vm::vm_interface* m){
            m->halt();
            std::cout << "exit" << std::endl;
            m->set_register(RegAlias::a0, 0);
        }));
    }
    machine.init_isa();
    machine.init_memory();
    auto ok = machine.set_program(code);
    if (!ok)
    {
        std::cerr << "unable load program(no memory)" << std::endl;
        return;
    }
    machine.start();
    machine.set_register(vm::RegAlias::sp, vm::basic_vm::def_data_size);
    machine.run();
}

void run_vm(const fs::path &program_file, bool debug)
{
    auto bin = load_program(program_file);

    if (bin)
    {
        run_vm(*bin, debug);
    }
}

void disasm(const fs::path &program_file)
{
    auto bin = load_program(program_file);

    if (bin)
    {
        disasm(*bin);
    }
}

void disasm(const program &code)
{
    using namespace std::literals;
    using op_type = vm::opcode::OpcodeBase;
    vm::registry registry;
    bool rv32i_ok = vm::rv32i::register_rv32i_set(&registry);
    bool rv32m_ok = vm::rv32m::register_rv32m_set(&registry);

    std::cout << std::boolalpha << "rv32i_ok = " << rv32i_ok << std::endl;
    std::cout << std::boolalpha << "rv32m_ok = " << rv32m_ok << std::endl;

    std::cout
        << std::setw(10) << std::left << "hex"
        << std::setw(10) << std::left << "group"
        << std::setw(10) << std::left << "instr"
        << std::setw(10) << std::left << "args"
        << std::endl;
    for (size_t i = 0; i < code.size(); i+= sizeof(op_type)) {
        const auto *p = code.data() + i;
        auto *op = reinterpret_cast<const op_type*>(p);
        auto handler = registry.find_handler(op);
        auto mnemonic = handler ? handler->get_mnemonic() : "UNKNOWN"sv;
        auto args = handler ? handler->get_args(op) : "UNKNOWN"s;
        std::cout << std::hex
                  << std::setw(8) << std::setfill('0') << std::right << op->code
                  << "  "
                  << std::setw(10) << std::setfill(' ') << std::left << vm::opcode::get_op_id(static_cast<vm::opcode::OpcodeType>(op->get_code()))
                  << std::setw(10) << std::setfill(' ') << std::left << mnemonic
                  << std::setw(10) << std::setfill(' ') << std::left << args
                  << std::endl;
    }
}
