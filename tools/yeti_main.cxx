#include <yeti-vm/vm_shared.hxx>

#include "yeti-vm/vm_opcode.hxx"
#include "yeti-vm/vm_handler.hxx"
#include "yeti-vm/vm_basic.hxx"
#include "yeti-vm/vm_handlers_rv32i.hxx"
#include "yeti-vm/vm_handlers_rv32m.hxx"
#include "yeti-vm/vm_base_types.hxx"
#include "yeti-vm/vm_utility.hxx"

#include <iostream>
#include <variant>

namespace fs = std::filesystem;

struct load_helper
{
    using bin_data = vm::program_code_t;
    using hex_data = vm::hex_file;
    using bin_opt = std::optional<bin_data>;
    using hex_opt = std::optional<hex_data>;
    using program_data = std::variant<bin_data, hex_data, std::nullptr_t>;

    [[nodiscard]]
    bool load_file(const fs::path& fileName)
    {
        const auto ext = fileName.extension();
        data = nullptr;

        if (ext == ".bin")
        {
            auto code = vm::load_program(fileName);
            if (code)
                data = code.value();
        }
        else if (ext == ".hex")
        {
            auto code = vm::parse_hex(fileName);
            if (code)
                data = code.value();
        }

        return is_bin() || is_hex();
    }

    [[nodiscard]]
    const auto * as_bin() const
    {
        return std::get_if<bin_data>(&data);
    }
    [[nodiscard]]
    const auto * as_hex() const
    {
        return std::get_if<hex_data>(&data);
    }

    [[nodiscard]]
    bool is_bin() const
    {
        return holds_alternative<bin_data>(data);
    }
    [[nodiscard]]
    bool is_hex() const
    {
        return holds_alternative<hex_data>(data);
    }
    [[nodiscard]]
    bool is_null() const
    {
        return holds_alternative<std::nullptr_t>(data);
    }

    [[nodiscard]]
    bool is_empty() const
    {
        bool empty = is_null();

        empty |= is_bin() && as_bin()->empty();
        empty |= is_hex() && as_hex()->empty();

        return empty;
    }

    [[nodiscard]]
    bool set_program(vm::basic_vm& vm, vm::basic_vm::address_t pc_value = 0) const
    {
        if (is_empty()) return false;
        if (is_bin()) return vm.set_program(*as_bin(), pc_value);
        if (is_hex()) return vm.set_program(*as_hex());
        return false;
    }

    program_data data = nullptr;
};

void disasm(const vm::program_code_t &code);

void run_vm(const load_helper &code, bool debug);

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cout << "Usage:" << std::endl;
        std::cout << "\texe d <path/to/file.bin> - disasm bin file" << std::endl;
        std::cout << "\texe <v|V> <path/to/program> - run 'bin' or 'hex' file." << std::endl;
        std::cout << "\t\tv - no debug output" << std::endl;
        std::cout << "\t\tV - enable debug output" << std::endl;
        return 0;
    }

    fs::path program_file = argv[2];

    load_helper helper;

    if (!helper.load_file(program_file))
    {
        std::cerr << "Unable load program from " << program_file;
        return EXIT_FAILURE;
    }
    if (helper.is_empty())
    {
        std::cerr << "Empty program file " << program_file;
        return EXIT_FAILURE;
    }

    switch(*argv[1])
    {
    case 'd':
        if (!helper.is_bin())
        {
            std::cerr << "unable disasm not '.bin' file" << std::endl;
            return EXIT_FAILURE;
        }
        disasm(*helper.as_bin());
        break;
    case 'v':
        run_vm(helper, false);
        break;
    case 'V':
        run_vm(helper, true);
        break;
    default:
        std::cout << "Unknown option: " << argv[1] << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void init_syscalls(vm::syscall_registry &sys);

void run_vm(const load_helper &code, bool debug)
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
    auto ok = code.set_program(machine, 0);
    if (!ok)
    {
        std::cerr << "unable load program(no memory)" << std::endl;
        return;
    }
    machine.start();
    try
    {
        machine.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception" << e.what() << std::endl;
        machine.dump_state(std::cerr);
        throw ;
    }
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

        for (vm::vm_interface::address_t i = 0; i < buff_sz; i+= sizeof(vm::register_t))
        {
            m->write_memory(buff_ptr + i, sizeof(vm::register_t), i * i);
        }

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
