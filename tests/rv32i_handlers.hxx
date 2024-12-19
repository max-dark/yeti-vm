#include "rv32i_mocks.hxx"

#include <yeti-vm/vm_opcode.hxx>
#include <yeti-vm/vm_handler.hxx>
#include <concepts>
#include <ranges>

namespace tests::rv32i
{
using vm::register_t;
using vm::register_no;

using GroupId = vm::opcode::OpcodeType;
using Format = vm::opcode::BaseFormat;
using vm::opcode::Decoder;
using vm::opcode::Encoder;
using Code = vm::opcode::opcode_t;
using ExtId = Code;

using RegId = vm::register_no;
using RegAlias = vm::RegAlias;
using Address = vm::vm_interface::address_t;
using Offset = vm::vm_interface::offset_t;


template<class Handler>
concept Implementation = std::is_base_of_v<vm::interface, Handler>;
using HandlerPtr = std::unique_ptr<vm::interface>;

class RV32I_Handler : public ::testing::Test
{
protected:
    using r_bits = vm::bit_tools::bits<vm::register_t>;
    void SetUp() override
    {
    }

    void TearDown() override
    {
        ptr.reset();
    }

protected:
    static constexpr ExtId NoFuncA = vm::no_func_a;
    static constexpr ExtId NoFuncB = vm::no_func_b;

    template<Implementation Type>
    [[nodiscard]]
    vm::interface *create()
    {
        ptr = std::make_unique<Type>();
        return ptr.get();
    }

    [[nodiscard]]
    vm::interface *get() const
    {
        return ptr.get();
    }
    static vm::InstructionId make_id(GroupId groupId, Format format,
                                     ExtId extA = NoFuncA,
                                     ExtId extB = NoFuncB)
    {
        return {
            groupId, format,
            extA, extB
        };
    }

private:
    HandlerPtr ptr;
};

} // namespace tests::rv32i
