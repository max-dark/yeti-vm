#include <gtest/gtest.h>

#include <yeti-vm/vm_opcode.hxx>

using vm::opcode::Encoder;
using vm::opcode::Decoder;

class RV32_Encode_Registers: public ::testing::TestWithParam<vm::register_no> {};

INSTANTIATE_TEST_SUITE_P(
        Registers
        , RV32_Encode_Registers
        , ::testing::Range<vm::register_no>(0, vm::register_count)
        );

TEST_P(RV32_Encode_Registers, EncodeRD)
{
    vm::register_no id = GetParam();
    auto value = Encoder::encode_rd(id);
    Decoder parser{value};

    ASSERT_EQ(id, parser.get_rd());
}

TEST_P(RV32_Encode_Registers, EncodeRS1)
{
    vm::register_no id = GetParam();
    auto value = Encoder::encode_rs1(id);
    Decoder parser{value};

    ASSERT_EQ(id, parser.get_rs1());
}

TEST_P(RV32_Encode_Registers, EncodeRS2)
{
    vm::register_no id = GetParam();
    auto value = Encoder::encode_rs2(id);
    Decoder parser{value};

    ASSERT_EQ(id, parser.get_rs2());
}
