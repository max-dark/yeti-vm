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
        return make_id(GroupId::OP_IMM, Format::R_TYPE, funcA, funcB);
    }

    using TestStep = std::function<Code(Code lhs, Code rhs)>;
    static void commonTest(const vm::interface* impl, Code funcA, Code funcB, const TestStep& step)
    {
        ASSERT_TRUE(impl->get_id().equal(expectedId(funcA, funcA)));

        for (RegId dest = 0; dest < vm::register_count; ++dest)
        {
            for (Offset values: {-8, -4, 0, +4, +8})
            {
                MockVM mockVm;

                RegId id_dst = 0;
                RegId id_lhs = 0;
                RegId id_rhs = 0;

                Code lhs = 0;
                Code rhs = 0;
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
    FAIL() << "Not implemented";
}

TEST_F(RV32I_Handler_RR, Sub)
{
    FAIL() << "Not implemented";
}

TEST_F(RV32I_Handler_RR, Sll)
{
    FAIL() << "Not implemented";
}

TEST_F(RV32I_Handler_RR, Slt)
{
    FAIL() << "Not implemented";
}

TEST_F(RV32I_Handler_RR, SltU)
{
    FAIL() << "Not implemented";
}

TEST_F(RV32I_Handler_RR, Xor)
{
    FAIL() << "Not implemented";
}

TEST_F(RV32I_Handler_RR, Srl)
{
    FAIL() << "Not implemented";
}

TEST_F(RV32I_Handler_RR, Sra)
{
    FAIL() << "Not implemented";
}

TEST_F(RV32I_Handler_RR, Or)
{
    FAIL() << "Not implemented";
}

TEST_F(RV32I_Handler_RR, And)
{
    FAIL() << "Not implemented";
}

} // namespace tests::rv32i
