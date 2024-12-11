#include "vm_stdlib.hxx"

#include <vm_opcode.hxx>
#include <vm_handler.hxx>
#include <vm_basic.hxx>
#include <vm_handlers_rv32i.hxx>
#include <vm_handlers_rv32m.hxx>
#include <vm_base_types.hxx>
#include <vm_utility.hxx>

#include <iostream>

namespace fs = std::filesystem;

void disasm(const vm::program_code_t &code);

void run_vm(const vm::program_code_t &code, bool debug);

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cout << "Usage: exe <v|V|d> <path/to/file.bin>" << std::endl;
        return 0;
    }

    fs::path program_file = argv[2];

    auto program = vm::load_program(program_file);

    if (!program)
    {
        std::cerr << "Unable load program from " << program_file;
        return EXIT_FAILURE;
    }
    if (program->empty())
    {
        std::cerr << "Empty program file " << program_file;
        return EXIT_FAILURE;
    }

    switch(*argv[1])
    {
    case 'd':
        disasm(program.value());
        break;
    case 'v':
        run_vm(program.value(), false);
        break;
    case 'V':
        run_vm(program.value(), true);
        break;
    default:
        std::cout << "Unknown option: " << argv[1] << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void init_syscalls(vm::syscall_registry &sys);

void run_vm(const vm::program_code_t &code, bool debug)
{
    vm::basic_vm machine;

    machine.enable_debugging(debug);

    init_syscalls(machine.get_syscalls());
    bool isa_ok = machine.init_isa();
    bool mem_ok = machine.init_memory();
    bool init_ok = isa_ok && mem_ok;
    if (!init_ok)
    {
        std::cerr
            << std::format("Unable init VM: isa = {} / mem = {}", isa_ok, mem_ok)
            << std::endl;
        return;
    }
    auto ok = machine.set_program(code, 0);
    if (!ok)
    {
        std::cerr << "unable load program(no memory)" << std::endl;
        return;
    }
    machine.start();
    machine.run();
}

void init_syscalls(vm::syscall_registry &sys)
{
    using vm::RegAlias;
    using call = vm::syscall_functor;
    sys.register_handler(call::create(1024, "open", [](vm::vm_interface* m){
        auto name_ptr = m->get_register(vm::a0);
        auto flags = m->get_register(vm::a1);
        std::cout << "open " << vm::to_signed(name_ptr) << " " << flags << std::endl;
        m->set_register(vm::a0, 0);
    }));
    sys.register_handler(call::create(63, "read", [](vm::vm_interface* m){
        auto file_id = m->get_register(vm::a0);
        auto buff_ptr = m->get_register(vm::a1);
        auto buff_sz = m->get_register(vm::a2);
        std::cout << "read " << file_id << " " << vm::to_signed(buff_ptr) << " " << buff_sz << std::endl;
        m->set_register(vm::a0, 0);
    }));
    sys.register_handler(call::create(57, "close", [](vm::vm_interface* m){
        auto file_id = m->get_register(vm::a0);
        std::cout << "close " << file_id << std::endl;
        m->set_register(vm::a0, 0);
    }));
    sys.register_handler(call::create(11, "put_char", [](vm::vm_interface* m){
        auto value = m->get_register(vm::a0);
        auto data = reinterpret_cast<const char*>(&value);
        std::cout << *data;
        m->set_register(vm::a0, 0);
    }));
    sys.register_handler(call::create(1, "put_int", [](vm::vm_interface* m){
        auto value = m->get_register(vm::a0);
        std::cout << vm::to_signed(value);
        m->set_register(vm::a0, 0);
    }));
    sys.register_handler(call::create(10, "exit", [](vm::vm_interface* m){
        m->halt();
        std::cout << "exit" << std::endl;
        m->set_register(vm::a0, 0);
    }));
}

void disasm(const vm::program_code_t &code)
{
    using namespace std::literals;
    using op_type = vm::opcode::OpcodeBase;
    vm::registry registry;
    bool rv32i_ok = vm::rv32i::register_rv32i_set(&registry);
    bool rv32m_ok = vm::rv32m::register_rv32m_set(&registry);

    std::cout << std::boolalpha << "rv32i_ok = " << rv32i_ok << std::endl;
    std::cout << std::boolalpha << "rv32m_ok = " << rv32m_ok << std::endl;

    std::cout
        << std::setw(10) << std::left << "addr"
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
                  << std::setw(8) << std::setfill('0') << std::right << i
                  << "  "
                  << std::setw(8) << std::setfill('0') << std::right << op->code
                  << "  "
                  << std::setw(10) << std::setfill(' ') << std::left << vm::opcode::get_op_id(static_cast<vm::opcode::OpcodeType>(op->get_code()))
                  << std::setw(10) << std::setfill(' ') << std::left << mnemonic
                  << std::setw(10) << std::setfill(' ') << std::left << args
                  << std::endl;
    }
}
