#include "rv32i_handlers.hxx"
#include <yeti-vm/vm_handlers_rv32i.hxx>

namespace tests::rv32i
{
using namespace vm::rv32i;

using ::testing::_;
using ::testing::Return;
using ::testing::Sequence;

/**
 * @brief Integer Register-Register instructions(OP / R-type)
 *
 * SLL, SRL, and SRA perform logical left, logical right, and arithmetic right shifts
 * on the value in register rs1 by the shift amount held in the lower 5 bits of register rs2.
 */
class RV32I_Handler_RR
        : public RV32I_Handler
{
protected:
    static Decoder encode(RegId dest, RegId lhs, RegId rhs, Code funcA, Code funcB)
    {
        Code code = Encoder::r_type(
                GroupId::OP
                , dest, lhs, rhs
                , funcA, funcB
                );
        return Decoder{ code };
    }

    static vm::InstructionId expectedId(Code funcA, Code funcB)
    {
        return make_id(GroupId::OP, Format::R_TYPE, funcA, funcB);
    }

    using TestStep = std::function<Code(Code lhs, Code rhs)>;
    /// TODO: use threads(?)
    static void commonTest(const vm::interface* impl, Code funcA, Code funcB, const TestStep& step)
    {
        ASSERT_TRUE(impl->get_id().equal(expectedId(funcA, funcB)));

        for (RegId id_dst = 0; id_dst < vm::register_count; ++id_dst)
        for (RegId id_lhs = 0; id_lhs < vm::register_count; ++id_lhs)
        for (RegId id_rhs = 0; id_rhs < vm::register_count; ++id_rhs)
        {
            for (Offset value: std::views::iota(-7) | std::views::take(15))
            {
                Sequence calculate;
                MockVM mockVm;

                // TODO: values for (id_lhs == id_rhs) should be same (?)
                Code lhs = value * id_dst + (id_lhs * id_rhs);
                Code rhs = value * id_dst - (id_lhs * id_rhs);
                Code res = step(lhs, rhs);

                Decoder code = encode(id_dst, id_lhs, id_rhs, funcA, funcB);

                ASSERT_EQ(code.get_code(), GroupId::OP);
                ASSERT_EQ(code.get_rd (), id_dst);
                ASSERT_EQ(code.get_rs1(), id_lhs);
                ASSERT_EQ(code.get_rs2(), id_rhs);
                ASSERT_EQ(code.get_func3(), funcA);
                ASSERT_EQ(code.get_func7(), funcB);

                EXPECT_CALL(mockVm, get_register(id_lhs))
                    .InSequence(calculate)
                    .WillOnce(Return(lhs))
                ;

                EXPECT_CALL(mockVm, get_register(id_rhs))
                    .InSequence(calculate)
                    .WillOnce(Return(rhs))
                ;

                EXPECT_CALL(mockVm, set_register(id_dst, res))
                    .InSequence(calculate)
                    .WillOnce(Return())
                ;

                impl->exec(&mockVm, &code);
            }
        }
    }
};

TEST_F(RV32I_Handler_RR, Add)
{
    TestStep step = [](Code lhs, Code rhs) -> Code
    {
        return lhs + rhs;
    };

    return commonTest(create<add_r>(), 0b0'000, 0b0'0'00000, step);
}

TEST_F(RV32I_Handler_RR, Sub)
{
    TestStep step = [](Code lhs, Code rhs) -> Code
    {
        return lhs - rhs;
    };

    return commonTest(create<sub_r>(), 0b0'000, 0b0'1'00000, step);
}

TEST_F(RV32I_Handler_RR, Sll)
{
    TestStep step = [](Code lhs, Code rhs) -> Code
    {
        return lhs << r_bits::get_bits<0, 5>(rhs);
    };

    return commonTest(create<sll_r>(), 0b0'001, 0b0'0'00000, step);
}

TEST_F(RV32I_Handler_RR, Slt)
{
    TestStep step = [](Code lhs, Code rhs) -> Code
    {
        return (r_bits::to_signed(lhs) < r_bits::to_signed(rhs)) ? 1 : 0;
    };

    return commonTest(create<slt_r>(), 0b0'010, 0b0'0'00000, step);
}

TEST_F(RV32I_Handler_RR, SltU)
{
    TestStep step = [](Code lhs, Code rhs) -> Code
    {
        return (r_bits::to_unsigned(lhs) < r_bits::to_unsigned(rhs)) ? 1 : 0;
    };

    return commonTest(create<sltu_r>(), 0b0'011, 0b0'0'00000, step);
}

TEST_F(RV32I_Handler_RR, Xor)
{
    TestStep step = [](Code lhs, Code rhs) -> Code
    {
        return lhs ^ rhs;
    };

    return commonTest(create<xor_r>(), 0b0'100, 0b0'0'00000, step);
}

TEST_F(RV32I_Handler_RR, Srl)
{
    TestStep step = [](Code lhs, Code rhs) -> Code
    {
        return lhs >> r_bits::get_bits<0, 5>(rhs);
    };

    return commonTest(create<srl_r>(), 0b0'101, 0b0'0'00000, step);
}

TEST_F(RV32I_Handler_RR, Sra)
{
    TestStep step = [](Code lhs, Code rhs) -> Code
    {
        return r_bits::to_signed(lhs) >> r_bits::get_bits<0, 5>(rhs);
    };

    return commonTest(create<sra_r>(), 0b0'101, 0b0'1'00000, step);
}

TEST_F(RV32I_Handler_RR, Or)
{
    TestStep step = [](Code lhs, Code rhs) -> Code
    {
        return lhs | rhs;
    };

    return commonTest(create<or_r>(), 0b0'110, 0b0'0'00000, step);
}

TEST_F(RV32I_Handler_RR, And)
{
    TestStep step = [](Code lhs, Code rhs) -> Code
    {
        return lhs & rhs;
    };

    return commonTest(create<and_r>(), 0b0'111, 0b0'0'00000, step);
}

} // namespace tests::rv32i
