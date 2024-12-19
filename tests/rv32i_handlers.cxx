#include "rv32_vm_mocks.hxx"
#include "rv32i_handlers.hxx"

#include <yeti-vm/vm_opcode.hxx>
#include <yeti-vm/vm_handlers_rv32i.hxx>

#include <concepts>

namespace tests::rv32i
{
using namespace std::literals;
using namespace vm::rv32i;
using vm::register_t;
using vm::register_no;

using vm::opcode::get_op_id;
using vm::opcode::get_code_id;

TEST_F(RV32I_Handler, CreateAndGet)
{
    EXPECT_EQ(get(), nullptr);
    auto ptr = create<lui>();
    EXPECT_NE(ptr, nullptr);
    EXPECT_EQ(ptr, get());
}

struct TestImpl
{
    using TestInfo = ::testing::TestParamInfo<TestImpl>;
    std::string_view id;
    vm::interface::ptr instance;
    std::string_view mnemonic;
    GroupId code;

    static std::string to_string(const TestInfo& info)
    {
        auto& param = info.param;
        std::string id = std::format("{}_impl_for_{}_{}"
                           , info.index
                           , param.mnemonic
                           , param.id  // TODO: demangle param.id
                           );
        std::replace_if(id.begin(), id.end()
                        , [](int c) { return ! std::isalnum(c); }
                        , '_');
        return id;
    }
};

template<Implementation Impl>
TestImpl impl(std::string_view id, GroupId code)
{
    return {
        .id = typeid(Impl).name(),
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
              impl<lui>("lui", GroupId::LUI)
            , impl<auipc>("auipc", GroupId::AUIPC)
            , impl<jalr>("jalr", GroupId::JALR)

            , impl<beq>("beq", GroupId::BRANCH)
            , impl<bne>("bne", GroupId::BRANCH)
            , impl<blt>("blt", GroupId::BRANCH)
            , impl<bge>("bge", GroupId::BRANCH)

            , impl<bltu>("bltu", GroupId::BRANCH)
            , impl<bgeu>("bgeu", GroupId::BRANCH)

            , impl<lb>("lb", GroupId::LOAD)
            , impl<lh>("lh", GroupId::LOAD)
            , impl<lw>("lw", GroupId::LOAD)
            , impl<lbu>("lbu", GroupId::LOAD)
            , impl<lhu>("lhu", GroupId::LOAD)

            , impl<sb>("sb", GroupId::STORE)
            , impl<sh>("sh", GroupId::STORE)
            , impl<sw>("sw", GroupId::STORE)

            , impl<addi>("addi", GroupId::OP_IMM)
            , impl<slti>("slti", GroupId::OP_IMM)
            , impl<sltiu>("sltiu", GroupId::OP_IMM)

            , impl<xori>("xori", GroupId::OP_IMM)
            , impl<ori> ("ori" , GroupId::OP_IMM)
            , impl<andi>("andi", GroupId::OP_IMM)
            , impl<slli>("slli", GroupId::OP_IMM)
            , impl<srli>("srli", GroupId::OP_IMM)
            , impl<srai>("srai", GroupId::OP_IMM)

            , impl<add_r>("add", GroupId::OP)
            , impl<sub_r>("sub", GroupId::OP)
            , impl<sll_r>("sll", GroupId::OP)
            , impl<slt_r>("slt", GroupId::OP)
            , impl<sltu_r>("sltu", GroupId::OP)
            , impl<xor_r>("xor", GroupId::OP)
            , impl<srl_r>("srl", GroupId::OP)
            , impl<sra_r>("sra", GroupId::OP)
            , impl<or_r>("or", GroupId::OP)
            , impl<and_r>("and", GroupId::OP)

            , impl<fence>("fence", GroupId::MISC_MEM)
            , impl<fence_i>("fence.i", GroupId::MISC_MEM)

            , impl<env_call>("env", GroupId::SYSTEM)

            , impl<csrrw>("csrrw", GroupId::SYSTEM)
            , impl<csrrs>("csrrs", GroupId::SYSTEM)
            , impl<csrrc>("csrrc", GroupId::SYSTEM)

            , impl<csrrwi>("csrrwi", GroupId::SYSTEM)
            , impl<csrrsi>("csrrsi", GroupId::SYSTEM)
            , impl<csrrci>("csrrci", GroupId::SYSTEM)
        );
    }
};

INSTANTIATE_TEST_SUITE_P(
        HandlerISA
        , RV32I_Handler_ISA
        , RV32I_Handler_ISA::make_values()
        , TestImpl::to_string
);

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
