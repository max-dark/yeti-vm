#include "rv32i_handlers.hxx"
#include <yeti-vm/vm_handlers_rv32i.hxx>

namespace tests::rv32i
{
using namespace vm::rv32i;

using ::testing::_;
using ::testing::Return;

/**
 * @brief Integer Register-Register instructions(OP / R-type)
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
    static void commonTest(const vm::interface* impl, Code funcA, Code funcB, const TestStep& step)
    {
        ASSERT_TRUE(impl->get_id().equal(expectedId(funcA, funcB)));

        for (RegId dest = 0; dest < vm::register_count; ++dest)
        {
            for (Offset value: {-8, -4, 0, +4, +8})
            {
                MockVM mockVm;

                RegId id_dst = dest;
                RegId id_lhs = 0b01'1111 & ((value * - dest) % vm::register_count);
                RegId id_rhs = 0b01'1111 & ((value * + dest) % vm::register_count);

                Code lhs = value + dest;
                Code rhs = value - dest;
                Code res = step(lhs, rhs);

                Decoder code = encode(id_dst, id_lhs, id_rhs, funcA, funcB);

                EXPECT_CALL(mockVm, get_register(id_lhs))
                    .WillRepeatedly(Return(lhs));
                EXPECT_CALL(mockVm, get_register(id_rhs))
                        .WillRepeatedly(Return(rhs));
                EXPECT_CALL(mockVm, set_register(id_dst, res));

                impl->exec(&mockVm, &code);
            }
        }
    }
};

TEST_F(RV32I_Handler_RR, Add)
{
    TestStep step = [](Code lhs, Code rhs) -> Code
    {
        return -1;
    };

    return commonTest(create<add_r>(), 0b0'000, 0b0'0'00000, step);
}

TEST_F(RV32I_Handler_RR, Sub)
{
    TestStep step = [](Code lhs, Code rhs) -> Code
    {
        return -1;
    };

    return commonTest(create<sub_r>(), 0b0'000, 0b0'1'00000, step);
}

TEST_F(RV32I_Handler_RR, Sll)
{
    TestStep step = [](Code lhs, Code rhs) -> Code
    {
        return -1;
    };

    return commonTest(create<sll_r>(), 0b0'001, 0b0'0'00000, step);
}

TEST_F(RV32I_Handler_RR, Slt)
{
    TestStep step = [](Code lhs, Code rhs) -> Code
    {
        return -1;
    };

    return commonTest(create<slt_r>(), 0b0'010, 0b0'0'00000, step);
}

TEST_F(RV32I_Handler_RR, SltU)
{
    TestStep step = [](Code lhs, Code rhs) -> Code
    {
        return -1;
    };

    return commonTest(create<sltu_r>(), 0b0'011, 0b0'0'00000, step);
}

TEST_F(RV32I_Handler_RR, Xor)
{
    TestStep step = [](Code lhs, Code rhs) -> Code
    {
        return -1;
    };

    return commonTest(create<sll_r>(), 0b0'100, 0b0'0'00000, step);
}

TEST_F(RV32I_Handler_RR, Srl)
{
    TestStep step = [](Code lhs, Code rhs) -> Code
    {
        return -1;
    };

    return commonTest(create<srl_r>(), 0b0'101, 0b0'0'00000, step);
}

TEST_F(RV32I_Handler_RR, Sra)
{
    TestStep step = [](Code lhs, Code rhs) -> Code
    {
        return -1;
    };

    return commonTest(create<sra_r>(), 0b0'101, 0b0'1'00000, step);
}

TEST_F(RV32I_Handler_RR, Or)
{
    TestStep step = [](Code lhs, Code rhs) -> Code
    {
        return -1;
    };

    return commonTest(create<or_r>(), 0b0'110, 0b0'0'00000, step);
}

TEST_F(RV32I_Handler_RR, And)
{
    TestStep step = [](Code lhs, Code rhs) -> Code
    {
        return -1;
    };

    return commonTest(create<and_r>(), 0b0'111, 0b0'0'00000, step);
}

} // namespace tests::rv32i
