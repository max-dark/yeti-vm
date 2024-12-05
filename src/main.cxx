#include "stdlib.hxx"

#include <vm_base_types.hxx>
#include <vm_utility.hxx>
#include <vm_opcode.hxx>
#include <vm_handler.hxx>

#include <iostream>
#include <format>

namespace fs = std::filesystem;

using program = std::vector<std::uint8_t>;

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

namespace vm
{

struct basic_vm: public registry
{
    using code_memory_t = std::vector<std::uint8_t>;
    using data_memory_t = std::vector<std::uint8_t>;
    using address_t = std::uint32_t;
    using offset_t = std::int32_t;

    void halt()
    {
        running = false;
    }

    void jump_abs(address_t dest)
    {
        bool valid = dest % sizeof(opcode::opcode_t);
        valid = valid && (dest < code.size());
        if (!valid) halt();
        set_pc(dest);
    }

    void jump_to(offset_t value)
    {
        auto dest = get_pc() + value;
        bool valid = dest % sizeof(opcode::opcode_t);
        jump_abs(dest);
    }

    void jump_if(bool condition, offset_t value)
    {
        if (condition) jump_to(value);
        else inc_pc();
    }

    void jump_if_abs(bool condition, address_t value)
    {
        if (condition) jump_abs(value);
        else inc_pc();
    }

    void syscall()
    {
        inc_pc();
    }

    void debug()
    {
        inc_pc();
    }

    void control()
    {
        inc_pc();
    }

    void barrier()
    {
        inc_pc();
    }

    void read_memory(address_t from, uint8_t size, register_t& value)
    {
        auto ptr = get_ptr(from);
        if (!ptr)
        {
            halt();
            return;
        }
        if (size > sizeof(value))
        {
            halt();
            return;
        }
        std::memcpy(&value, ptr, size);
        inc_pc();
    }

    void write_memory(address_t from, uint8_t size, register_t value)
    {
        auto ptr = get_ptr(from);
        if (!ptr)
        {
            halt();
            return;
        }
        if (size > sizeof(value))
        {
            halt();
            return;
        }
        std::memcpy(ptr, &value, size);
        inc_pc();
    }

    void set_register(register_no r, register_t value)
    {
        if (r > 0 && r <= register_count)
        {
            registers[r] = value;
        }
    }

    [[nodiscard]]
    register_t get_register(register_no r) const
    {
        if (r > 0 && r <= register_count)
        {
            return registers[r];
        }
        return 0;
    }

    [[nodiscard]]
    register_t get_pc() const
    {
        return get_register(RegAlias::pc);
    }
    void set_pc(register_t value)
    {
        if (value + sizeof(opcode::opcode_t) >= code.size())
        {
            halt();
            return;
        }
        set_register(RegAlias::pc, value);
    }
    void inc_pc()
    {
        set_pc(get_pc() + sizeof(opcode::opcode_t));
    }

    [[nodiscard]]
    const void* get_ptr(address_t address) const
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

    [[nodiscard]]
    void* get_ptr(address_t address)
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

    void set_ro_size(size_t size)
    {
        code.resize(size);
    }

    void set_rw_size(size_t size)
    {
        data.resize(size);
    }

    void run_step()
    {
        auto* current = get_current();
        auto handler = find_handler(current);
        if (!handler)
        {
            halt();
            return;
        }
        handler->exec(this, current);
    }

    void run()
    {
        while (is_running())
        {
            run_step();
        }
    }

    void start()
    {
        std::fill(registers.begin(), registers.end(), 0);
        std::fill(data.begin(), data.end(), 0);
        running = true;
    }

    bool is_running() const
    {
        return running;
    }

    void init_memory()
    {
        init_memory(def_code_size, def_data_size);
    }

    void init_memory(size_t code_size, size_t data_size)
    {
        set_ro_size(code_size);
        set_rw_size(data_size);
    }

    bool set_program(const program& bin)
    {
        if (!is_initialized()) return false;
        if (bin.size() > code.size()) return false;

        std::copy(bin.begin(), bin.end(), code.begin());

        return true;
    }

    static constexpr address_t def_code_base = 0;
    static constexpr address_t def_data_base = 4 * 1024 * 1024;

    static constexpr size_t def_code_size = 4 * 1024 * 1024;
    static constexpr size_t def_data_size = 4 * 1024 * 1024;

    bool is_initialized() const
    {
        return !(code.empty() || data.empty());
    }
private:
    [[nodiscard]]
    const opcode::OpcodeBase* get_current() const
    {
        const auto * ptr = code.data() + get_pc();
        return reinterpret_cast<const opcode::OpcodeBase*>(ptr);
    }
    register_file registers{};
    code_memory_t code; // ro memory
    data_memory_t data; // rw memory
    address_t code_base = def_code_base;
    address_t data_base = def_data_base;

    bool running = false;
};

namespace rv32i
{

struct lui: public instruction_base<opcode::LUI, opcode::U_TYPE> {
    [[nodiscard]]
    std::string get_args(const opcode::OpcodeBase* code) const override
    {
        std::string args{get_register_alias(code->get_rd())};
        return args + ", " + opcode::to_hex(code->decode_u());
    }
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "lui";
    }
    void exec(basic_vm *vm, const opcode::OpcodeBase* current) const override
    {
        auto dest = current->get_rd();
        auto data = current->decode_u();

        vm->set_register(dest, data);
    }
};
struct auipc: public instruction_base<opcode::AUIPC, opcode::U_TYPE> {
    [[nodiscard]]
    std::string get_args(const opcode::OpcodeBase* code) const override
    {
        std::string args{get_register_alias(code->get_rd())};
        return args + ", " + opcode::to_hex(code->decode_u());
    }
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "auipc";
    }
    void exec(basic_vm *vm, const opcode::OpcodeBase* current) const override
    {
        auto dest = current->get_rd();
        auto data = current->decode_u();
        auto pc_value = vm->get_pc();
        vm->set_register(dest, data + pc_value);
    }
};

struct jal: public instruction_base<opcode::JAL, opcode::J_TYPE> {
    static opcode::signed_t get_data(const opcode::OpcodeBase* code)
    {
        auto value = opcode::extend_sign(code->decode_j(), code->code);
        return std::bit_cast<opcode::signed_t>(value);
    }
    [[nodiscard]]
    std::string get_args(const opcode::OpcodeBase* code) const override
    {
        std::string args{get_register_alias(code->get_rd())};
        return args + ", " + std::to_string(get_data(code));
    }
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "jal";
    }
    void exec(basic_vm *vm, const opcode::OpcodeBase* current) const override
    {
        auto dest = current->get_rd();
        auto offset = get_data(current);
        auto pc_value = vm->get_pc();
        vm->set_register(dest, pc_value + sizeof(opcode::opcode_t));
        vm->jump_to(offset); // @see 2.5.1 "Unconditional jumps"
    }
};
struct jalr: public instruction_base<opcode::JALR, opcode::I_TYPE> {
    static opcode::signed_t get_data(const opcode::OpcodeBase* code)
    {
        auto value = opcode::extend_sign(code->decode_i(), code->code);
        return std::bit_cast<opcode::signed_t>(value);
    }
    [[nodiscard]]
    std::string get_args(const opcode::OpcodeBase* code) const override
    {
        std::string dest{get_register_alias(code->get_rd())};
        std::string src{get_register_alias(code->get_rs1())};
        return dest + ", " + src + ", " + std::to_string(get_data(code));
    }
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "jalr";
    }
    void exec(basic_vm *vm, const opcode::OpcodeBase* current) const override
    {
        auto dest = current->get_rd();
        auto src = current->get_rs1();
        auto offset = get_data(current);
        auto pc_value = vm->get_pc();
        auto base = vm->get_register(src);

        vm->set_register(dest, pc_value + sizeof(opcode::opcode_t));
        vm->jump_abs((base + offset) & (~1u)); // @see 2.5.1 "Unconditional jumps"
    }
};

template<opcode::opcode_t Type>
struct branch: public instruction_base<opcode::JALR, opcode::B_TYPE, Type> {
    [[nodiscard]]
    std::string get_args(const opcode::OpcodeBase* code) const override
    {
        std::string lhs{get_register_alias(code->get_rs1())};
        std::string rhs{get_register_alias(code->get_rs2())};
        return lhs + ", " + rhs + ", " + std::to_string(get_data(code));
    }

    [[nodiscard]]
    static opcode::signed_t get_data(const opcode::OpcodeBase* current)
    {
        auto value = opcode::extend_sign(current->decode_b(), current->code);
        return std::bit_cast<opcode::signed_t>(value);
    }

    [[nodiscard]]
    virtual bool compare(register_t lhs, register_t rhs) const = 0;
    void exec(basic_vm *vm, const opcode::OpcodeBase* current) const override
    {
        auto lhs = vm->get_register(current->get_rs1());
        auto rhs = vm->get_register(current->get_rs2());
        auto offset = get_data(current);

        vm->jump_if(compare(lhs, rhs), offset);
    }
};

struct beq : branch<0b0000> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "beq";
    }
    [[nodiscard]]
    bool compare(register_t lhs, register_t rhs) const override
    {
        return to_signed(lhs) == to_signed(rhs);
    }
};
struct bne : branch<0b0001> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "bne";
    }
    [[nodiscard]]
    bool compare(register_t lhs, register_t rhs) const override
    {
        return to_signed(lhs) != to_signed(rhs);
    }
};
struct blt : branch<0b0100> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "blt";
    }
    [[nodiscard]]
    bool compare(register_t lhs, register_t rhs) const override
    {
        return to_signed(lhs) < to_signed(rhs);
    }
};
struct bge : branch<0b0101> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "bge";
    }
    [[nodiscard]]
    bool compare(register_t lhs, register_t rhs) const override
    {
        return lhs >= to_signed(rhs);
    }
};
struct bltu: branch<0b0110> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "bltu";
    }
    [[nodiscard]]
    bool compare(register_t lhs, register_t rhs) const override
    {
        return lhs < rhs;
    }
};
struct bgeu: branch<0b0111> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "bgeu";
    }
    [[nodiscard]]
    bool compare(register_t lhs, register_t rhs) const override
    {
        return lhs >= rhs;
    }
};

void add_branch(registry* r)
{
    r->register_handler<beq>();
    r->register_handler<bne>();
    r->register_handler<blt>();
    r->register_handler<bge>();
    r->register_handler<bltu>();
    r->register_handler<bgeu>();
}

template<opcode::opcode_t Type>
struct load: public instruction_base<opcode::LOAD, opcode::I_TYPE, Type> {
    static signed_t get_offset(const opcode::OpcodeBase* current)
    {
        return to_signed(current->decode_i());
    }
    static basic_vm::address_t get_address(basic_vm *vm, const opcode::OpcodeBase* current)
    {
        auto base = vm->get_register(current->get_rs1());
        return base + get_offset(current);
    }
    [[nodiscard]]
    std::string get_args(const opcode::OpcodeBase* code) const override
    {
        std::string dest{get_register_alias(code->get_rd())};
        std::string base{get_register_alias(code->get_rs1())};
        return dest + ", " + base + ", " + std::to_string(get_offset(code));
    }
    virtual register_t read_memory(basic_vm* vm, basic_vm::address_t address) const = 0;
    void exec(basic_vm *vm, const opcode::OpcodeBase* current) const override
    {
        auto address = get_address(vm, current);
        auto value = read_memory(vm, address);
        auto dest = current->get_rd();
        vm->set_register(dest, value);
    }
};

struct lb : load<0b0000> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "lb";
    }
    register_t read_memory(basic_vm* vm, basic_vm::address_t address) const override
    {
        register_t value = 0;
        vm->read_memory(address, 1, value);
        if (value & opcode::mask_value<7, 1>)
            return opcode::mask_value<8, 24> | value;
        return value;
    }
};
struct lh : load<0b0001> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "lh";
    }
    register_t read_memory(basic_vm* vm, basic_vm::address_t address) const override
    {
        register_t value = 0;
        vm->read_memory(address, 2, value);
        if (value & opcode::mask_value<15, 1>)
            return opcode::mask_value<16, 16> | value;
        return value;
    }
};
struct lw : load<0b0010> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "lw";
    }
    register_t read_memory(basic_vm* vm, basic_vm::address_t address) const override
    {
        register_t value = 0;
        vm->read_memory(address, 4, value);
        return value;
    }
};
struct lbu: load<0b0100> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "lbu";
    }
    register_t read_memory(basic_vm* vm, basic_vm::address_t address) const override
    {
        register_t value = 0;
        vm->read_memory(address, 1, value);
        return value;
    }
};
struct lhu: load<0b0101> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "lhu";
    }
    register_t read_memory(basic_vm* vm, basic_vm::address_t address) const override
    {
        register_t value = 0;
        vm->read_memory(address, 2, value);
        return value;
    }
};

void add_load(registry* r)
{
    r->register_handler<lb>();
    r->register_handler<lh>();
    r->register_handler<lw>();
    r->register_handler<lbu>();
    r->register_handler<lhu>();
}

template<opcode::opcode_t Type>
struct store: public instruction_base<opcode::STORE, opcode::S_TYPE, Type> {
    static signed_t get_offset(const opcode::OpcodeBase* current)
    {
        return to_signed(current->decode_s());
    }
    static basic_vm::address_t get_address(basic_vm *vm, const opcode::OpcodeBase* current)
    {
        auto base = vm->get_register(current->get_rs1());
        return base + get_offset(current);
    }
    [[nodiscard]]
    std::string get_args(const opcode::OpcodeBase* code) const override
    {
        std::string base{get_register_alias(code->get_rs1())};
        std::string src{get_register_alias(code->get_rs2())};
        return src + ", " + base + ", " + std::to_string(get_offset(code));
    }
    void exec(basic_vm *vm, const opcode::OpcodeBase* current) const override
    {
        auto value = vm->get_register(current->get_rs2());
        auto address = get_address(vm, current);
        write_memory(vm, address, value);
    }
    virtual void write_memory(basic_vm* vm, basic_vm::address_t address, register_t value) const = 0;
};

struct sb: store<0b0000> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "sb";
    }
    void write_memory(basic_vm* vm, basic_vm::address_t address, register_t value) const override
    {
        vm->write_memory(address, 1, value);
    }
};
struct sh: store<0b0001> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "sh";
    }
    void write_memory(basic_vm* vm, basic_vm::address_t address, register_t value) const override
    {
        vm->write_memory(address, 2, value);
    }
};
struct sw: store<0b0010> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "sw";
    }
    void write_memory(basic_vm* vm, basic_vm::address_t address, register_t value) const override
    {
        vm->write_memory(address, 4, value);
    }
};

void add_store(registry* r)
{
    r->register_handler<sb>();
    r->register_handler<sh>();
    r->register_handler<sw>();
}

template<opcode::opcode_t Type>
struct int_imm: public instruction_base<opcode::OP_IMM, opcode::I_TYPE, Type> {
    static signed_t get_data(const opcode::OpcodeBase* current)
    {
        return to_signed(opcode::extend_sign(current->decode_i(), current->code));
    }
    [[nodiscard]]
    std::string get_args(const opcode::OpcodeBase* code) const override
    {
        std::string dest{get_register_alias(code->get_rd())};
        std::string src{get_register_alias(code->get_rs1())};
        return dest + ", " + src + ", " + std::to_string(get_data(code));
    }
};

struct addi : int_imm<0b0000> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "addi";
    }
    void exec(basic_vm *vm, const opcode::OpcodeBase* current) const override
    {
        auto dest = current->get_rd();
        auto src  = vm->get_register(current->get_rs1());
        auto value = src + get_data(current);
        vm->set_register(dest, value);
    }
};

struct sli : int_imm<0b0010> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "sli";
    }
    void exec(basic_vm *vm, const opcode::OpcodeBase* current) const override
    {
        auto dest = current->get_rd();
        auto value = vm->get_register(current->get_rs1());
        auto src  = to_signed(value);
        auto data = get_data(current);
        if (src < data)
        {
            vm->set_register(dest, to_unsigned(data));
        }
        else
        {
            vm->set_register(dest, 0);
        }
    }
};
struct sliu: int_imm<0b0011> {
    [[nodiscard]]
    std::string get_args(const opcode::OpcodeBase* code) const override
    {
        std::string dest{get_register_alias(code->get_rd())};
        std::string src{get_register_alias(code->get_rs1())};
        return dest + ", " + src + ", " + std::to_string(code->decode_i());
    }
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "sliu";
    }
    void exec(basic_vm *vm, const opcode::OpcodeBase* current) const override
    {
        auto dest = current->get_rd();
        auto value = vm->get_register(current->get_rs1());
        auto data = current->decode_i();
        if (value < data)
        {
            vm->set_register(dest, data);
        }
        else
        {
            vm->set_register(dest, 0);
        }
    }
};
struct xori: int_imm<0b0100> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "xori";
    }
    void exec(basic_vm *vm, const opcode::OpcodeBase* current) const override
    {
        auto dest = current->get_rd();
        auto value = vm->get_register(current->get_rs1());
        auto data = get_data(current);
        vm->set_register(dest, value ^ data);
    }
};
struct ori : int_imm<0b0110> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "ori";
    }
    void exec(basic_vm *vm, const opcode::OpcodeBase* current) const override
    {
        auto dest = current->get_rd();
        auto value = vm->get_register(current->get_rs1());
        auto data = get_data(current);
        vm->set_register(dest, value | data);
    }
};
struct andi: int_imm<0b0111> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "andi";
    }
    void exec(basic_vm *vm, const opcode::OpcodeBase* current) const override
    {
        auto dest = current->get_rd();
        auto value = vm->get_register(current->get_rs1());
        auto data = get_data(current);
        vm->set_register(dest, value & data);
    }
};

void add_int_imm(registry* r)
{
    r->register_handler<addi>();
    r->register_handler<sli>();
    r->register_handler<sliu>();
    r->register_handler<xori>();
    r->register_handler<ori>();
    r->register_handler<andi>();
}

template<opcode::opcode_t Type, opcode::opcode_t Variant>
struct shift_imm: public instruction_base<opcode::OP_IMM, opcode::R_TYPE, Type, (Variant << 5)> {
    static register_t get_data(const opcode::OpcodeBase* current)
    {
        return current->decode_i() & opcode::mask_value<0, 5>;
    }
    [[nodiscard]]
    std::string get_args(const opcode::OpcodeBase* code) const override
    {
        std::string dest{get_register_alias(code->get_rd())};
        std::string src{get_register_alias(code->get_rs1())};
        return dest + ", " + src + ", " + std::to_string(get_data(code));
    }
};

struct slli: shift_imm<0b0001, 0> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "slli";
    }
    void exec(basic_vm *vm, const opcode::OpcodeBase* current) const override
    {
        auto dest = current->get_rd();
        auto value = vm->get_register(current->get_rs1());
        auto data = get_data(current);
        vm->set_register(dest, value << data);
    }
};
struct srli: shift_imm<0b0101, 0> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "srli";
    }
    void exec(basic_vm *vm, const opcode::OpcodeBase* current) const override
    {
        auto dest = current->get_rd();
        auto value = vm->get_register(current->get_rs1());
        auto data = get_data(current);
        vm->set_register(dest, value >> data);
    }
};
struct srai: shift_imm<0b0101, 1> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "srai";
    }
    void exec(basic_vm *vm, const opcode::OpcodeBase* current) const override
    {
        auto dest = current->get_rd();
        auto value = vm->get_register(current->get_rs1());
        auto data = get_data(current);
        vm->set_register(dest, opcode::extend_sign(value >> data, value));
    }
};

void add_shift_imm(registry* r)
{
    r->register_handler<slli>();
    r->register_handler<srli>();
    r->register_handler<srai>();
}

template<opcode::opcode_t Type, opcode::opcode_t Variant>
struct int_r: public instruction_base<opcode::OP, opcode::R_TYPE, Type, (Variant << 5)> {
    [[nodiscard]]
    std::string get_args(const opcode::OpcodeBase* code) const override
    {
        std::string dest{get_register_alias(code->get_rd())};
        std::string lhs{get_register_alias(code->get_rs1())};
        std::string rhs{get_register_alias(code->get_rs2())};
        return dest + ", " + lhs + ", " + rhs;
    }
    [[nodiscard]]
    virtual register_t calculate(register_t lhs, register_t rhs) const = 0;
    void exec(basic_vm *vm, const opcode::OpcodeBase* current) const override
    {
        auto dest = current->get_rd();
        auto lhs = vm->get_register(current->get_rs1());
        auto rhs = vm->get_register(current->get_rs2());

        vm->set_register(dest, calculate(lhs, rhs));
    }
};

struct add_r : int_r<0b0000, 0> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "add";
    }
    [[nodiscard]]
    register_t calculate(register_t lhs, register_t rhs) const override
    {
        return lhs + rhs;
    }
};
struct sub_r : int_r<0b0000, 1> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "sub";
    }
    [[nodiscard]]
    register_t calculate(register_t lhs, register_t rhs) const override
    {
        return lhs - rhs;
    }
};
struct sll_r : int_r<0b0001, 0> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "sll";
    }
    [[nodiscard]]
    register_t calculate(register_t lhs, register_t rhs) const override
    {
        return lhs << rhs;
    }
};
struct slt_r : int_r<0b0010, 0> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "slt";
    }
    [[nodiscard]]
    register_t calculate(register_t lhs, register_t rhs) const override
    {
        return to_signed(lhs) < to_signed(rhs);
    }
};
struct sltu_r: int_r<0b0011, 0> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "sltu";
    }
    [[nodiscard]]
    register_t calculate(register_t lhs, register_t rhs) const override
    {
        return lhs < rhs;
    }
};
struct xor_r : int_r<0b0100, 0> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "xor";
    }
    [[nodiscard]]
    register_t calculate(register_t lhs, register_t rhs) const override
    {
        return lhs ^ rhs;
    }
};
struct srl_r : int_r<0b0101, 0> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "srl";
    }
    [[nodiscard]]
    register_t calculate(register_t lhs, register_t rhs) const override
    {
        return lhs >> rhs;
    }
};
struct sra_r : int_r<0b0101, 1> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "sra";
    }
    [[nodiscard]]
    register_t calculate(register_t lhs, register_t rhs) const override
    {
        return opcode::extend_sign(lhs >> rhs, lhs);
    }
};
struct or_r  : int_r<0b0110, 0> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "or";
    }
    [[nodiscard]]
    register_t calculate(register_t lhs, register_t rhs) const override
    {
        return lhs | rhs;
    }
};
struct and_r : int_r<0b0111, 0> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "and";
    }
    [[nodiscard]]
    register_t calculate(register_t lhs, register_t rhs) const override
    {
        return lhs & rhs;
    }
};

void add_int(registry* r)
{
    r->register_handler<add_r>();
    r->register_handler<sub_r>();
    r->register_handler<sll_r>();
    r->register_handler<sll_r>();
    r->register_handler<slt_r>();
    r->register_handler<sltu_r>();
    r->register_handler<xor_r>();
    r->register_handler<srl_r>();
    r->register_handler<sra_r>();
    r->register_handler<or_r>();
    r->register_handler<and_r>();
}

template<opcode::opcode_t Type>
struct misc_mem: public instruction_base<opcode::MISC_MEM, opcode::I_TYPE, Type> {};

struct fence  : misc_mem<0b0000> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "fence";
    }
    void exec(basic_vm *vm, const opcode::OpcodeBase* current) const override
    {
        vm->barrier();
    }
};
struct fence_i: misc_mem<0b0001> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "fence.i";
    }
    void exec(basic_vm *vm, const opcode::OpcodeBase* current) const override
    {
        vm->barrier();
    }
};

void add_misc(registry* r)
{
    r->register_handler<fence>();
    r->register_handler<fence_i>();
}

// ECALL / EBREAK
struct env_call: public instruction_base<opcode::SYSTEM, opcode::I_TYPE> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "env(call/break)";
    }
    void exec(basic_vm *vm, const opcode::OpcodeBase* current) const override
    {
        if (current->decode_i())
        {
            vm->debug();
        }
        else
        {
            vm->syscall();
        }
    }
};

template<opcode::opcode_t Type>
struct csr: public instruction_base<opcode::SYSTEM, opcode::I_TYPE, Type> {
    void exec(basic_vm *vm, const opcode::OpcodeBase* current) const override
    {
        vm->control();
    }
};

struct csrrw : csr<0b0001> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "csrrw";
    }
};
struct csrrs : csr<0b0010> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "csrrs";
    }
};
struct csrrc : csr<0b0011> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "csrrc";
    }
};
struct csrrwi: csr<0b0101> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "csrrwi";
    }
};
struct csrrsi: csr<0b0110> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "csrrsi";
    }
};
struct csrrci: csr<0b0111> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "csrrci";
    }
};

void add_system(registry* r)
{
    r->register_handler<env_call>();
    r->register_handler<csrrw>();
    r->register_handler<csrrs>();
    r->register_handler<csrrc>();
    r->register_handler<csrrwi>();
    r->register_handler<csrrsi>();
    r->register_handler<csrrci>();
}

void add_all(registry* r)
{
    r->register_handler<lui>();
    r->register_handler<auipc>();
    r->register_handler<jal>();
    r->register_handler<jalr>();

    add_branch(r);
    add_load(r);
    add_store(r);
    add_int_imm(r);
    add_shift_imm(r);
    add_int(r);
    add_misc(r);
    add_system(r);
}

} // namespace rv32i

} // namespace vm

void disasm(const fs::path &program_file);
void disasm(const program &code);

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cout << "Usage: exe <path/to/file.bin>" << std::endl;
        return 0;
    }

    fs::path program_file = argv[1];
    disasm(program_file);

    return 0;
}

void run_vm(const program &code)
{
    vm::basic_vm machine;
    vm::rv32i::add_all(&machine);

    machine.init_memory();
    auto ok = machine.set_program(code);
    if (!ok)
    {
        std::cerr << "unable load program(no memory)" << std::endl;
        return;
    }
    machine.start();
    machine.run();
}
void run_vm(const fs::path &program_file)
{
    auto bin = load_program(program_file);

    if (bin)
    {
        run_vm(*bin);
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
    vm::rv32i::add_all(&registry);

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
