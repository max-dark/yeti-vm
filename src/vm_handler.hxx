#pragma once

#include <vm_base_types.hxx>
#include <vm_opcode.hxx>

namespace vm
{
struct vm_interface;

inline constexpr opcode::opcode_t no_func_a = 1 << 4;
inline constexpr opcode::opcode_t no_func_b = 1 << 8;
struct InstructionId
{
    opcode::opcode_t code = 0;
    opcode::BaseFormat format = opcode::UNKNOWN;
    opcode::opcode_t funcA = no_func_a;
    opcode::opcode_t funcB = no_func_b;

    bool operator<(const InstructionId& rhs) const
    {
        bool result = code < rhs.code;
        //result = result && (format < rhs.format);
        result = result && (funcA < rhs.funcA);
        result = result && (funcB < rhs.funcB);
        return result;
    }
};

struct interface
{
    using ptr = std::shared_ptr<interface>;
    virtual ~interface() = default;

    [[nodiscard]]
    virtual const InstructionId& get_id() const = 0;

    [[nodiscard]]
    virtual opcode::opcode_t get_code_base() const = 0;
    [[nodiscard]]
    virtual opcode::opcode_t get_func_a() const = 0;
    [[nodiscard]]
    virtual opcode::opcode_t get_func_b() const = 0;
    [[nodiscard]]
    virtual std::string_view get_mnemonic() const = 0;
    [[nodiscard]]
    virtual std::string get_args(const opcode::OpcodeBase* code) const
    {
        return get_args(code->code);
    }
    [[nodiscard]]
    virtual std::string get_args(opcode::opcode_t code) const = 0;
    [[nodiscard]]
    virtual opcode::BaseFormat get_type() const = 0;

    virtual void exec(vm_interface* vm, const opcode::OpcodeBase* current) const = 0;
};

template
<
        opcode::opcode_t CodeBase,
        opcode::BaseFormat Format,
        opcode::opcode_t FuncA = no_func_a,
        opcode::opcode_t FuncB = no_func_b
>
struct instruction_base : public interface
{
    [[nodiscard]]
    const InstructionId& get_id() const final
    {
        static const InstructionId id{
                .code = CodeBase,
                .format = Format,
                .funcA = FuncA,
                .funcB = FuncB
        };

        return id;
    }

    [[nodiscard]]
    opcode::opcode_t get_code_base() const final
    {
        return CodeBase;
    }

    [[nodiscard]]
    opcode::opcode_t get_func_a() const final
    {
        return FuncA;
    }

    [[nodiscard]]
    opcode::opcode_t get_func_b() const final
    {
        return FuncB;
    }

    [[nodiscard]]
    std::string_view get_mnemonic() const override
    {
        auto code = opcode::OpcodeType{get_code_base()};
        return opcode::get_op_id(code);
    }
    [[nodiscard]]
    std::string get_args(opcode::opcode_t code) const override
    {
        return opcode::to_hex(opcode::get_bits(code, 8, 32));
    }
    [[nodiscard]]
    opcode::BaseFormat get_type() const final
    {
        return Format;
    }
};

struct registry
{
    using handler_ptr = const interface*;
    using type_map = std::map<InstructionId, interface::ptr>;
    using base_map = std::map<opcode::opcode_t, type_map>;

    template<typename Handler>
    inline void register_handler()
    {
        static_assert(std::is_base_of_v<interface, Handler>, "Wrong type of Handler");
        register_handler(std::make_shared<Handler>());
    }
    void register_handler(interface::ptr handler);

    handler_ptr find_handler(const opcode::OpcodeBase* code) const;

    base_map handlers;
};


} // namespace vm
