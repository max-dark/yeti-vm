#pragma once

#include <vm_base_types.hxx>
#include <vm_opcode.hxx>
#include <vm_handler.hxx>
#include <vm_interface.hxx>
#include <vm_utility.hxx>

namespace vm::rv32m
{
using result_signed_t = std::int64_t;
using result_unsigned_t = std::uint64_t;
static constexpr result_unsigned_t result_mask = register_t{~0u};
static constexpr result_unsigned_t result_size = sizeof(register_t) * 8;

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

/// lower bits of (signed * signed)
struct mul: math<0b0000> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final { return "mul"; }
    [[nodiscard]]
    register_t calculate(register_t lhs, register_t rhs) const final
    {
        result_signed_t result = to_signed(lhs) * to_signed(rhs);
        return result & result_mask;
    }
};

/// upper bits of (signed * signed)
struct mulh: math<0b0001> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final { return "mulh"; }
    [[nodiscard]]
    register_t calculate(register_t lhs, register_t rhs) const final
    {
        result_signed_t result = to_signed(lhs) * to_signed(rhs);
        return (result >> result_size) & result_mask;
    }
};

/// upper bits of (signed * unsigned)
struct mulhsu: math<0b0010> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final { return "mulhsu"; }
    [[nodiscard]]
    register_t calculate(register_t lhs, register_t rhs) const final
    {
        result_signed_t result = to_signed(lhs) * rhs;
        return (result >> result_size) & result_mask;
    }
};

/// upper bits of (unsigned * unsigned)
struct mulhu: math<0b0011> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final { return "mulhu"; }
    [[nodiscard]]
    register_t calculate(register_t lhs, register_t rhs) const final
    {
        result_unsigned_t result = lhs * rhs;
        return (result >> result_size) & result_mask;
    }
};

/// (signed / signed)
struct div: math<0b0100> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final { return "div"; }
    [[nodiscard]]
    register_t calculate(register_t lhs, register_t rhs) const final
    {
        result_signed_t result = to_signed(lhs) / to_signed(rhs);
        return result & result_mask;
    }
};

/// (unsigned / unsigned)
struct divu: math<0b0101> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final { return "divu"; }
    [[nodiscard]]
    register_t calculate(register_t lhs, register_t rhs) const final
    {
        result_unsigned_t result = lhs / rhs;
        return result & result_mask;
    }
};

/// (signed % signed)
struct rem: math<0b0110> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final { return "rem"; }
    [[nodiscard]]
    register_t calculate(register_t lhs, register_t rhs) const final
    {
        result_signed_t result = to_signed(lhs) % to_signed(rhs);
        return result & result_mask;
    }
};

/// (unsigned % unsigned)
struct remu: math<0b0111> {
    [[nodiscard]]
    std::string_view get_mnemonic() const final { return "remu"; }
    [[nodiscard]]
    register_t calculate(register_t lhs, register_t rhs) const final
    {
        result_unsigned_t result = lhs % rhs;
        return result & result_mask;
    }
};

/// register RV32M set in registry
bool register_rv32m_set(registry* r);
} // namespace vm::rv32m
