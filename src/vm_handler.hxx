/// instruction handler interfaces
#pragma once

#include <vm_base_types.hxx>
#include <vm_opcode.hxx>

namespace vm
{
struct vm_interface;

/// function ID for "no function"
inline constexpr opcode::opcode_t no_func_a = 1 << 4;
/// function ID for "no function"
inline constexpr opcode::opcode_t no_func_b = 1 << 8;

/// Instruction ID
struct InstructionId
{
    /// basic OPCODE
    opcode::opcode_t code = 0;
    /// encoding format
    opcode::BaseFormat format = opcode::UNKNOWN;
    /// function ID
    opcode::opcode_t funcA = no_func_a;
    /// function ID
    opcode::opcode_t funcB = no_func_b;

    /// comparator for std::map
    bool operator<(const InstructionId& rhs) const
    {
        bool result = code < rhs.code;
        //result = result && (format < rhs.format);
        result = result && (funcA < rhs.funcA);
        result = result && (funcB < rhs.funcB);
        return result;
    }
};

/// Handler interface
struct interface
{
    using ptr = std::shared_ptr<interface>;
    virtual ~interface() = default;

    /// get ID of instruction handled by this object
    [[nodiscard]]
    virtual const InstructionId& get_id() const = 0;

    /// get opcode group ID
    [[nodiscard]]
    virtual opcode::opcode_t get_code_base() const = 0;

    /// get opcode "func A" ID
    [[nodiscard]]
    virtual opcode::opcode_t get_func_a() const = 0;
    /// get opcode "func B" ID
    [[nodiscard]]
    virtual opcode::opcode_t get_func_b() const = 0;
    /// get instruction mnemonic
    [[nodiscard]]
    virtual std::string_view get_mnemonic() const = 0;
    /// disasm arguments
    [[nodiscard]]
    virtual std::string get_args(const opcode::OpcodeBase* code) const
    {
        return get_args(code->code);
    }
    /// disasm arguments
    [[nodiscard]]
    virtual std::string get_args(opcode::opcode_t code) const = 0;
    /// get encoding type
    [[nodiscard]]
    virtual opcode::BaseFormat get_type() const = 0;

    /**
     * execute instruction
     * @param vm pointer to VM implementation
     * @param current pointer to current instruction
     */
    virtual void exec(vm_interface* vm, const opcode::OpcodeBase* current) const = 0;
};

/**
 * generic implementation of OPCODE handler
 * @tparam CodeBase OPCODE group ID
 * @tparam Format encoding format
 * @tparam FuncA "func A" ID
 * @tparam FuncB "func B" ID
 */
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

/**
 * registry of instruction handlers
 */
struct registry
{
    using handler_ptr = const interface*;
    using type_map = std::map<InstructionId, interface::ptr>;
    using base_map = std::map<opcode::opcode_t, type_map>;

    /**
     * register handler by type
     * @tparam Handler
     */
    template<typename Handler>
    inline void register_handler()
    {
        static_assert(std::is_base_of_v<interface, Handler>, "Wrong type of Handler");
        register_handler(std::make_shared<Handler>());
    }
    /// register handler by pointer
    void register_handler(interface::ptr handler);

    /// find handler by instruction code
    handler_ptr find_handler(const opcode::OpcodeBase* code) const;

    /// handlers container
    base_map handlers;
};


} // namespace vm
