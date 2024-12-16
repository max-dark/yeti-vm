#include "rv32i_mocks.hxx"

#include <yeti-vm/vm_opcode.hxx>
#include <yeti-vm/vm_handlers_rv32i.hxx>

#include <concepts>

namespace tests::rv32i
{
using namespace vm::rv32i;
using vm::register_t;
using vm::register_no;

using vm::opcode::get_op_id;
using vm::opcode::get_code_id;

using Enum = vm::opcode::OpcodeType;
using Type = vm::opcode::OpcodeBase;

template<class Handler>
concept Implementation = std::is_base_of_v<vm::interface, Handler>;
using HandlerPtr = std::unique_ptr<vm::interface>;


class RV32I_Handler : public ::testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
        ptr.reset();
    }

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

private:
    HandlerPtr ptr;
};

TEST_F(RV32I_Handler, CreateAndGet)
{
    EXPECT_EQ(get(), nullptr);
    auto ptr = create<lui>();
    EXPECT_NE(ptr, nullptr);
    EXPECT_EQ(ptr, get());
}

struct TestImpl
{
    vm::interface::ptr instance;
    std::string_view mnemonic;
    Enum code;
};

template<Implementation Impl>
TestImpl impl(std::string_view id, Enum code)
{
    return {
        .instance = std::make_shared<Impl>(),
        .mnemonic = id,
        .code = code
    };
};

struct RV32I_Handler_ISA
        : public ::testing::TestWithParam<TestImpl>
{
    static auto make_values()
    {
        return ::testing::Values(
              impl<lui>("lui", Enum::LUI)
            , impl<auipc>("auipc", Enum::AUIPC)
            , impl<jalr>("jalr", Enum::JALR)

            , impl<beq>("beq", Enum::BRANCH)
            , impl<bne>("bne", Enum::BRANCH)
            , impl<blt>("blt", Enum::BRANCH)
            , impl<bge>("bge", Enum::BRANCH)

            , impl<bltu>("bltu", Enum::BRANCH)
            , impl<bgeu>("bgeu", Enum::BRANCH)
        );
    }
};

INSTANTIATE_TEST_SUITE_P(HandlerISA, RV32I_Handler_ISA, RV32I_Handler_ISA::make_values());

TEST_P(RV32I_Handler_ISA, Mnemonics)
{
    auto& p = GetParam();
    auto& impl = p.instance;
    ASSERT_EQ(impl->get_mnemonic(), p.mnemonic);
}

TEST_P(RV32I_Handler_ISA, CodeGroup)
{
    auto& p = GetParam();
    auto& impl = p.instance;
    ASSERT_EQ(impl->get_code_base(), p.code)
        << "Expected: " << get_op_id(p.code)
        << " / Actual: "  << get_code_id(impl->get_code_base());
}

} // namespace tests::rv32i
