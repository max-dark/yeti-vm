#pragma once

#include <vm_base_types.hxx>
#include <vm_opcode.hxx>
#include <vm_handler.hxx>
#include <vm_interface.hxx>

namespace vm::rv32m
{

template<opcode::opcode_t Type>
struct math: public instruction_base<opcode::OP, opcode::R_TYPE, Type, 0b000'0001> {
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

struct mul: math<0b0000> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final { return "mul"; }
    [[nodiscard]]
    register_t calculate(register_t lhs, register_t rhs) const final { return 0; }
};
struct mulh: math<0b0000> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final { return "mulh"; }
    [[nodiscard]]
    register_t calculate(register_t lhs, register_t rhs) const final { return 0; }
};
struct mulhsu: math<0b0001> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final { return "mulhsu"; }
    [[nodiscard]]
    register_t calculate(register_t lhs, register_t rhs) const final { return 0; }
};
struct mulhu: math<0b0010> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final { return "mulhu"; }
    [[nodiscard]]
    register_t calculate(register_t lhs, register_t rhs) const final { return 0; }
};
struct div: math<0b0011> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final { return "div"; }
    [[nodiscard]]
    register_t calculate(register_t lhs, register_t rhs) const final { return 0; }
};
struct divu: math<0b0100> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final { return "divu"; }
    [[nodiscard]]
    register_t calculate(register_t lhs, register_t rhs) const final { return 0; }
};
struct rem: math<0b0101> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final { return "rem"; }
    [[nodiscard]]
    register_t calculate(register_t lhs, register_t rhs) const final { return 0; }
};
struct remu: math<0b0111> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final { return "remu"; }
    [[nodiscard]]
    register_t calculate(register_t lhs, register_t rhs) const final { return 0; }
};

/// register RV32M set in registry
void register_rv32m_set(registry* r);
} // namespace vm::rv32m
