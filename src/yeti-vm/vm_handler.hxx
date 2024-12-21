/// instruction handler interfaces
#pragma once

#include "vm_base_types.hxx"
#include "vm_opcode.hxx"

namespace vm
{
struct vm_interface;

/// Instruction ID
struct InstructionId
{
    using GroupId = opcode::opcode_t;
    using Format = opcode::BaseFormat;
    using FunctionId = opcode::opcode_t;

    /// function ID for "no function"
    static constexpr FunctionId NoFuncA = 1 << 4;
    /// function ID for "no function"
    static constexpr FunctionId NoFuncB = 1 << 8;

private:
    using InternalId = opcode::opcode_t;

    /// basic OPCODE group
    const GroupId groupId = 0;
    /// encoding format
    const Format format = Format::UNKNOWN;
    /// function ID / A
    const FunctionId funcA = NoFuncA;
    /// function ID / B
    const FunctionId funcB = NoFuncB;

    const InternalId id = 0;
public:
    constexpr
    InstructionId(
            GroupId group,
            Format fmt,
            FunctionId func_a,
            FunctionId func_b
        )
        : groupId{group}
        , format{fmt}
        , funcA{func_a}
        , funcB{func_b}
        , id{ groupId | (funcA << 8) | (funcB << 16) }
    {}
    /// comparator for std::map
    friend constexpr auto operator<=>(const InstructionId& lhs, const InstructionId& rhs) noexcept
    {
        return lhs.id <=> rhs.id;
    }

    /// Hash function for std::unordered_map / std::unordered_set
    friend constexpr std::size_t hash_id(const InstructionId& key)
    {
        return key.id;
    }

    /// Strict EQ
    [[nodiscard]]
    constexpr bool equal(const InstructionId& rhs) const noexcept
    {
        return groupId == rhs.groupId
            && format == rhs.format
            && funcA == rhs.funcA
            && funcB == rhs.funcB
            ;
    }
};

/// function ID for "no function"
inline constexpr auto NoFuncA = InstructionId::NoFuncA;
/// function ID for "no function"
inline constexpr auto NoFuncB = InstructionId::NoFuncB;


/// Handler interface
struct HandlerInterface
{
    using ptr = std::shared_ptr<HandlerInterface>;
    virtual ~HandlerInterface() = default;

    /// get ID of instruction handled by this object
    [[nodiscard]]
    virtual const InstructionId& getId() const = 0;

    /// get opcode group ID
    [[nodiscard]]
    virtual opcode::opcode_t getGroupId() const = 0;

    /// get opcode "func A" ID
    [[nodiscard]]
    virtual opcode::opcode_t getFuncA() const = 0;
    /// get opcode "func B" ID
    [[nodiscard]]
    virtual opcode::opcode_t getFuncB() const = 0;
    /// get instruction mnemonic
    [[nodiscard]]
    virtual std::string_view mnemonic() const = 0;
    /// disassembly arguments
    [[nodiscard]]
    virtual std::string disassemblyArgs(const opcode::Decoder* code) const
    {
        return disassemblyArgs(code->code);
    }
    /// disassembly arguments
    [[nodiscard]]
    virtual std::string disassemblyArgs(opcode::opcode_t code) const = 0;
    /// get encoding type
    [[nodiscard]]
    virtual opcode::BaseFormat getEncodingFormat() const = 0;

    /**
     * execute instruction
     * @param vm pointer to VM implementation
     * @param current pointer to current instruction
     */
    virtual void exec(vm_interface* vm, const opcode::Decoder* current) const = 0;

    /**
     * skip PC increment
     */
    [[nodiscard]]
    virtual bool skip() const = 0;
};

/**
 * generic implementation of OPCODE handler
 * @tparam BaseGroupId OPCODE group ID
 * @tparam Format encoding format
 * @tparam FuncA "func A" ID
 * @tparam FuncB "func B" ID
 */
template
<
        InstructionId::GroupId BaseGroupId,
        InstructionId::Format Format,
        InstructionId::FunctionId FuncA = InstructionId::NoFuncA,
        InstructionId::FunctionId FuncB = InstructionId::NoFuncB
>
struct GenericHandler : public HandlerInterface
{
    [[nodiscard]]
    const InstructionId& getId() const final
    {
        static const InstructionId id{
                BaseGroupId, Format, FuncA, FuncB
        };

        return id;
    }

    [[nodiscard]]
    opcode::opcode_t getGroupId() const final
    {
        return BaseGroupId;
    }

    [[nodiscard]]
    opcode::opcode_t getFuncA() const final
    {
        return FuncA;
    }

    [[nodiscard]]
    opcode::opcode_t getFuncB() const final
    {
        return FuncB;
    }

    [[nodiscard]]
    std::string_view mnemonic() const override
    {
        auto code = opcode::OpcodeType{getGroupId()};
        return opcode::get_op_id(code);
    }
    [[nodiscard]]
    std::string disassemblyArgs(opcode::opcode_t code) const override
    {
        return opcode::to_hex(opcode::get_bits(code, 8, 32));
    }
    [[nodiscard]]
    opcode::BaseFormat getEncodingFormat() const final
    {
        return Format;
    }

    [[nodiscard]]
    bool skip() const override { return false; }
};

/**
 * registry of instruction handlers
 */
struct registry
{
    using handler_ptr = const HandlerInterface*;
    using handler_map = std::map<InstructionId, HandlerInterface::ptr>;

    /**
     * register handler by type
     * @tparam Handler
     */
    template<typename Handler>
    inline bool register_handler()
    {
        static_assert(std::is_base_of_v<HandlerInterface, Handler>, "Wrong type of Handler");
        return register_handler(std::make_shared<Handler>());
    }
    /// register handler by pointer
    bool register_handler(HandlerInterface::ptr handler);

    /// find handler by instruction code
    handler_ptr find_handler(const opcode::Decoder* code) const;

    /// handlers container
    handler_map handlers;
    /// mark that instruction have "func A"
    std::set<opcode::opcode_t> func_b;
    /// mark that instruction have "func B"
    std::set<opcode::opcode_t> func_a;
};


} // namespace vm
