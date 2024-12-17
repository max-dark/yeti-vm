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
 * load data from memory to RD
 * immediate encodes sign-extended offset from RS1
 * funcA encodes type of data
 *
 * rd = mem[rs1 + imm]
 */
class RV32I_Handler_Load
        : public RV32I_Handler
{
protected:
    static Decoder encode(RegId dest, RegId base, Offset offset, Code funcA)
    {
        return Decoder
        {
            Encoder::i_type(GroupId::LOAD
                                , dest, base,
                            bits<Offset>::to_unsigned(offset)
                                , funcA)
        };
    }

    static vm::InstructionId expectedId(Code funcA)
    {
        return make_id(GroupId::LOAD, Format::I_TYPE, funcA);
    }
};

TEST_F(RV32I_Handler_Load, ByteSigned)
{
    ASSERT_TRUE(false) << "Implement this";
}

TEST_F(RV32I_Handler_Load, HalfWordSigned)
{
    ASSERT_TRUE(false) << "Implement this";
}

TEST_F(RV32I_Handler_Load, Word)
{
    ASSERT_TRUE(false) << "Implement this";
}

TEST_F(RV32I_Handler_Load, ByteUnsigned)
{
    ASSERT_TRUE(false) << "Implement this";
}

TEST_F(RV32I_Handler_Load, HalfWordUnsigned)
{
    ASSERT_TRUE(false) << "Implement this";
}

} // namespace tests::rv32i
