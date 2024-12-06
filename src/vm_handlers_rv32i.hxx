#pragma once

#include <vm_base_types.hxx>
#include <vm_handler.hxx>
#include <vm_interface.hxx>
#include <vm_utility.hxx>

namespace vm::rv32i
{

/// load upper immediate
/// asm: lui dest, const
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
    void exec(vm_interface *vm, const opcode::OpcodeBase* current) const override
    {
        auto dest = current->get_rd();
        auto data = current->decode_u();

        vm->set_register(dest, data);
    }
};
/// add upper immediate to PC
/// asm: auipc dest, const
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
    void exec(vm_interface *vm, const opcode::OpcodeBase* current) const override
    {
        auto dest = current->get_rd();
        auto data = current->decode_u();
        auto pc_value = vm->get_pc();
        vm->set_register(dest, data + pc_value);
    }
};

/// jump and link, stores return address in dest
/// asm: jal dest, const
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
    void exec(vm_interface *vm, const opcode::OpcodeBase* current) const override
    {
        auto dest = current->get_rd();
        auto offset = get_data(current);
        auto pc_value = vm->get_pc();
        vm->set_register(dest, pc_value + sizeof(opcode::opcode_t));
        vm->jump_to(offset); // @see 2.5.1 "Unconditional jumps"
    }
};

/// jump and link by register
/// asm: jalr dest, src, const
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
    void exec(vm_interface *vm, const opcode::OpcodeBase* current) const override
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

/// branch (conditional jump)
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
    void exec(vm_interface *vm, const opcode::OpcodeBase* current) const override
    {
        auto lhs = vm->get_register(current->get_rs1());
        auto rhs = vm->get_register(current->get_rs2());
        auto offset = get_data(current);

        vm->jump_if(compare(lhs, rhs), offset);
    }
};

/// jump if eq
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
/// jump if not eq
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

/// jump if less
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

/// jump if greater or equal
struct bge : branch<0b0101> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "bge";
    }
    [[nodiscard]]
    bool compare(register_t lhs, register_t rhs) const override
    {
        return to_signed(lhs) >= to_signed(rhs);
    }
};

/// jump if less (unsigned)
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

/// jump if greater of equal
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

/// load(read) value from memory
template<opcode::opcode_t Type>
struct load: public instruction_base<opcode::LOAD, opcode::I_TYPE, Type> {
    static signed_t get_offset(const opcode::OpcodeBase* current)
    {
        return to_signed(current->decode_i());
    }
    static vm_interface::address_t get_address(vm_interface *vm, const opcode::OpcodeBase* current)
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
    virtual register_t read_memory(vm_interface* vm, vm_interface::address_t address) const = 0;
    void exec(vm_interface *vm, const opcode::OpcodeBase* current) const override
    {
        auto address = get_address(vm, current);
        auto value = read_memory(vm, address);
        auto dest = current->get_rd();
        vm->set_register(dest, value);
    }
};

/// load byte (sign extended)
struct lb : load<0b0000> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "lb";
    }
    register_t read_memory(vm_interface* vm, vm_interface::address_t address) const override
    {
        register_t value = 0;
        vm->read_memory(address, 1, value);
        if (value & opcode::mask_value<7, 1>)
            return opcode::mask_value<8, 24> | value;
        return value;
    }
};

/// load halfword (sign extended)
struct lh : load<0b0001> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "lh";
    }
    register_t read_memory(vm_interface* vm, vm_interface::address_t address) const override
    {
        register_t value = 0;
        vm->read_memory(address, 2, value);
        if (value & opcode::mask_value<15, 1>)
            return opcode::mask_value<16, 16> | value;
        return value;
    }
};

/// load word (sign extended)
struct lw : load<0b0010> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "lw";
    }
    register_t read_memory(vm_interface* vm, vm_interface::address_t address) const override
    {
        register_t value = 0;
        vm->read_memory(address, 4, value);
        return value;
    }
};

/// load byte (unsigned)
struct lbu: load<0b0100> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "lbu";
    }
    register_t read_memory(vm_interface* vm, vm_interface::address_t address) const override
    {
        register_t value = 0;
        vm->read_memory(address, 1, value);
        return value;
    }
};

/// load halfword (unsigned)
struct lhu: load<0b0101> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "lhu";
    }
    register_t read_memory(vm_interface* vm, vm_interface::address_t address) const override
    {
        register_t value = 0;
        vm->read_memory(address, 2, value);
        return value;
    }
};

/// store(write) value into memory
template<opcode::opcode_t Type>
struct store: public instruction_base<opcode::STORE, opcode::S_TYPE, Type> {
    static signed_t get_offset(const opcode::OpcodeBase* current)
    {
        return to_signed(current->decode_s());
    }
    static vm_interface::address_t get_address(vm_interface *vm, const opcode::OpcodeBase* current)
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
    void exec(vm_interface *vm, const opcode::OpcodeBase* current) const override
    {
        auto value = vm->get_register(current->get_rs2());
        auto address = get_address(vm, current);
        write_memory(vm, address, value);
    }
    virtual void write_memory(vm_interface* vm, vm_interface::address_t address, register_t value) const = 0;
};

/// store byte
struct sb: store<0b0000> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "sb";
    }
    void write_memory(vm_interface* vm, vm_interface::address_t address, register_t value) const override
    {
        vm->write_memory(address, 1, value);
    }
};

/// store halfword
struct sh: store<0b0001> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "sh";
    }
    void write_memory(vm_interface* vm, vm_interface::address_t address, register_t value) const override
    {
        vm->write_memory(address, 2, value);
    }
};

/// store word
struct sw: store<0b0010> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "sw";
    }
    void write_memory(vm_interface* vm, vm_interface::address_t address, register_t value) const override
    {
        vm->write_memory(address, 4, value);
    }
};

/// integer-immediate
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

/// add immediate
/// asm: addi rd, rs, const
struct addi : int_imm<0b0000> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "addi";
    }
    void exec(vm_interface *vm, const opcode::OpcodeBase* current) const override
    {
        auto dest = current->get_rd();
        auto src  = vm->get_register(current->get_rs1());
        auto value = src + get_data(current);
        vm->set_register(dest, value);
    }
};

/// set if less than immediate
/// asm: sli rd, rs, const
struct sli : int_imm<0b0010> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "sli";
    }
    void exec(vm_interface *vm, const opcode::OpcodeBase* current) const override
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

/// set if less than unsigned immediate
/// asm: sli rd, rs, const
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
    void exec(vm_interface *vm, const opcode::OpcodeBase* current) const override
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

/// xor
/// asm: xor rd, rs, const
struct xori: int_imm<0b0100> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "xori";
    }
    void exec(vm_interface *vm, const opcode::OpcodeBase* current) const override
    {
        auto dest = current->get_rd();
        auto value = vm->get_register(current->get_rs1());
        auto data = get_data(current);
        vm->set_register(dest, value ^ data);
    }
};

/// asm: or rd, rs, const
struct ori : int_imm<0b0110> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "ori";
    }
    void exec(vm_interface *vm, const opcode::OpcodeBase* current) const override
    {
        auto dest = current->get_rd();
        auto value = vm->get_register(current->get_rs1());
        auto data = get_data(current);
        vm->set_register(dest, value | data);
    }
};

/// asm: and rd, rs, const
struct andi: int_imm<0b0111> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "andi";
    }
    void exec(vm_interface *vm, const opcode::OpcodeBase* current) const override
    {
        auto dest = current->get_rd();
        auto value = vm->get_register(current->get_rs1());
        auto data = get_data(current);
        vm->set_register(dest, value & data);
    }
};

/// shift by immediate
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

/// asm: sll rd, rs, const
struct slli: shift_imm<0b0001, 0> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "slli";
    }
    void exec(vm_interface *vm, const opcode::OpcodeBase* current) const override
    {
        auto dest = current->get_rd();
        auto value = vm->get_register(current->get_rs1());
        auto data = get_data(current);
        vm->set_register(dest, value << data);
    }
};

/// asm: srl rd, rs, const
struct srli: shift_imm<0b0101, 0> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "srli";
    }
    void exec(vm_interface *vm, const opcode::OpcodeBase* current) const override
    {
        auto dest = current->get_rd();
        auto value = vm->get_register(current->get_rs1());
        auto data = get_data(current);
        vm->set_register(dest, value >> data);
    }
};

/// arithmetic shift
/// asm: sra rd, rs, const
struct srai: shift_imm<0b0101, 1> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "srai";
    }
    void exec(vm_interface *vm, const opcode::OpcodeBase* current) const override
    {
        auto dest = current->get_rd();
        auto value = vm->get_register(current->get_rs1());
        auto data = get_data(current);
        vm->set_register(dest, opcode::extend_sign(value >> data, value));
    }
};

/// integer-register
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
    void exec(vm_interface *vm, const opcode::OpcodeBase* current) const override
    {
        auto dest = current->get_rd();
        auto lhs = vm->get_register(current->get_rs1());
        auto rhs = vm->get_register(current->get_rs2());

        vm->set_register(dest, calculate(lhs, rhs));
    }
};

/// asm: add rd, rs1, rs2
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

/// asm: sub rd, rs1, rs2
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

/// asm: sll rd, rs1, rs2
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

/// set rd to 1 if rs1 < rs2, signed
/// asm: slt rd, rs1, rs2
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

/// set rd to 1 if rs1 < rs2, unsigned
/// asm: sltu rd, rs1, rs2
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

/// asm: xor rd, rs1, rs2
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

/// asm: srl rd, rs1, rs2
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

/// arithmetic shift
/// asm: sra rd, rs1, rs2
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

/// asm: or rd, rs1, rs2
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

/// asm: and rd, rs1, rs2
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

/// MISC-MEM group
template<opcode::opcode_t Type>
struct misc_mem: public instruction_base<opcode::MISC_MEM, opcode::I_TYPE, Type> {};

/// sync data memory
struct fence  : misc_mem<0b0000> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "fence";
    }
    void exec(vm_interface *vm, const opcode::OpcodeBase* current) const override
    {
        vm->barrier();
    }
};

/// sync instruction memory
struct fence_i: misc_mem<0b0001> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "fence.i";
    }
    void exec(vm_interface *vm, const opcode::OpcodeBase* current) const override
    {
        vm->barrier();
    }
};

// ECALL / EBREAK
struct env_call: public instruction_base<opcode::SYSTEM, opcode::I_TYPE, 0b0000> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "env(call/break)";
    }
    void exec(vm_interface *vm, const opcode::OpcodeBase* current) const override
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

/// CSR instructions
template<opcode::opcode_t Type>
struct csr: public instruction_base<opcode::SYSTEM, opcode::I_TYPE, Type> {
    void exec(vm_interface *vm, const opcode::OpcodeBase* current) const override
    {
        vm->control();
    }
};

/// atomic read and write
struct csrrw : csr<0b0001> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "csrrw";
    }
};

/// atomic read and set
struct csrrs : csr<0b0010> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "csrrs";
    }
};

/// atomic read and clear
struct csrrc : csr<0b0011> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "csrrc";
    }
};

/// unsigned(?) atomic read and write
struct csrrwi: csr<0b0101> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "csrrwi";
    }
};

/// unsigned(?) atomic read and set
struct csrrsi: csr<0b0110> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "csrrsi";
    }
};

/// unsigned(?) atomic read and clear
struct csrrci: csr<0b0111> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        return "csrrci";
    }
};

/// register RV32i set in registry
void register_rv32i_set(registry* r);

} // namespace vm::rv32i
