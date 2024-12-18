#include "rv32i_handlers.hxx"
#include <yeti-vm/vm_handlers_rv32i.hxx>

#include <ranges>

namespace tests::rv32i
{
using namespace vm::rv32i;

using vm::bit_tools::bits;
using ::testing::StrictMock;
using ::testing::_;
using ::testing::Args;
using ::testing::Return;

/**
 * branch performs jump if compare RS1 / RS2 return true
 * immediate is signed offset in multiples of 2 bytes (imm[0] == 0)
 *  addr = pc + imm
 *  throws exception if not aligned:
 *  ok = addr % 4 == 0
 */
class RV32I_Handler_Branch
        : public RV32I_Handler
        {
protected:
    static Decoder encode(RegId rs1, RegId rs2, Offset offset, Code funcA)
    {
        return Decoder{
            Encoder::b_type(GroupId::BRANCH
                            , rs1, rs2
                            , bits<Offset>::to_unsigned(offset)
                            , funcA)
        };
    }
    static vm::InstructionId expectedId(Code funcA)
    {
        return make_id(GroupId::BRANCH, Format::B_TYPE, funcA);
    }

    using CompareTest = void(MockVM&, vm::register_t, vm::register_t, Offset);
    static void branch(vm::interface* impl, Code funcA, CompareTest compareTest)
    {
        ASSERT_TRUE(impl->get_id().equal(expectedId(funcA)));

        for (Offset offset: { -8, -4, 0, +4, +8 })
        {
            for (RegId rs1 = 0; rs1 < vm::register_count; ++rs1)
            {
                for (RegId rs2 = 0; rs2 < vm::register_count; ++rs2)
                {
                    MockVM mock;
                    vm::register_t rs1_value = rs1 * offset;
                    vm::register_t rs2_value = rs2 * offset;
                    auto code = encode(rs1, rs2, offset, funcA);
                    EXPECT_CALL(mock, get_register(rs1))
                            .WillRepeatedly(Return(rs1_value));
                    EXPECT_CALL(mock, get_register(rs2))
                            .WillRepeatedly(Return(rs2_value));
                    compareTest(mock, rs1_value, rs2_value, offset);
                    impl->exec(&mock, &code);
                }
            }
        }
    }
};

TEST_F(RV32I_Handler_Branch, IfEqual)
{
    auto test =
    ([](MockVM &mock, vm::register_t rs1, vm::register_t rs2, Offset value)
    {
        EXPECT_CALL(mock, jump_if(rs1 == rs2, value));
    });
    constexpr Code funcA = 0b0000;
    branch(create<beq>(), funcA, test);
}

TEST_F(RV32I_Handler_Branch, IfNotEqual)
{
    auto test =
    ([](MockVM &mock, vm::register_t rs1, vm::register_t rs2, Offset value)
    {
        EXPECT_CALL(mock, jump_if(rs1 != rs2, value));
    });
    constexpr Code funcA = 0b0001;
    branch(create<bne>(), funcA, test);
}

TEST_F(RV32I_Handler_Branch, IfLessSigned)
{
    auto test = 
    ([](MockVM &mock, vm::register_t rs1, vm::register_t rs2, Offset value)
    {
        EXPECT_CALL(mock, jump_if(r_bits::to_signed(rs1) < r_bits::to_signed(rs2), value));
    });
    constexpr Code funcA = 0b0100;
    branch(create<blt>(), funcA, test);
}

TEST_F(RV32I_Handler_Branch, IfGreatOrEqualSigned)
{
    auto test =
    ([](MockVM &mock, vm::register_t rs1, vm::register_t rs2, Offset value)
    {
        EXPECT_CALL(mock, jump_if(r_bits::to_signed(rs1) >= r_bits::to_signed(rs2), value));
    });
    constexpr Code funcA = 0b0101;
    branch(create<bge>(), funcA,test);
}

TEST_F(RV32I_Handler_Branch, IfLessUnsigned)
{
    auto test =
    ([](MockVM &mock, vm::register_t rs1, vm::register_t rs2, Offset value)
    {
        EXPECT_CALL(mock, jump_if((rs1) < (rs2), value));
    });
    constexpr Code funcA = 0b0110;
    branch(create<bltu>(), funcA,test);
}

TEST_F(RV32I_Handler_Branch, IfGreatOrEqualUnsigned)
{
    auto test = 
    ([](MockVM &mock, vm::register_t rs1, vm::register_t rs2, Offset value)
    {
        EXPECT_CALL(mock, jump_if((rs1) >= (rs2), value));
    });
    constexpr Code funcA = 0b0111;
    branch(create<bgeu>(), funcA, test);
}

} // namespace tests::rv32i
