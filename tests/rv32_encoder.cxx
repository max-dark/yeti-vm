#include <gtest/gtest.h>

#include <yeti-vm/vm_opcode.hxx>

using vm::opcode::Encoder;
using vm::opcode::Decoder;

class RV32_Encode_GroupId: public ::testing::TestWithParam<vm::opcode::opcode_t> {};

INSTANTIATE_TEST_SUITE_P(
        Registers
        , RV32_Encode_GroupId
        , ::testing::Range<vm::opcode::opcode_t>(0b0000'0000, 0b1000'0000)
);

TEST_P(RV32_Encode_GroupId, SetGroupId)
{
    auto id = GetParam();
    auto value = Encoder::encode_group(id);
    Decoder parser{value};

    ASSERT_EQ(id, parser.get_code());
}

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

class RV32_Encode_FuncA: public ::testing::TestWithParam<vm::register_no> {};

INSTANTIATE_TEST_SUITE_P(
        FuncA
        , RV32_Encode_FuncA
        , ::testing::Range<vm::register_no>(0b0000, 0b1000)
);

TEST_P(RV32_Encode_FuncA, SetFuncA)
{
    vm::register_no id = GetParam();
    auto value = Encoder::encode_f3(id);
    Decoder parser{value};

    ASSERT_EQ(id, parser.get_func3());
}

class RV32_Encode_FuncB: public ::testing::TestWithParam<vm::register_no> {};

INSTANTIATE_TEST_SUITE_P(
        FuncB
        , RV32_Encode_FuncB
        , ::testing::Range<vm::register_no>(0b0000'0000, 0b1000'0000)
);

TEST_P(RV32_Encode_FuncB, SetFuncB)
{
    vm::register_no id = GetParam();
    auto value = Encoder::encode_f7(id);
    Decoder parser{value};

    ASSERT_EQ(id, parser.get_func7());
}

class RV32_Encode_Immediate_12: public ::testing::TestWithParam<vm::opcode::data_t> {};

INSTANTIATE_TEST_SUITE_P(
        Immediate
        , RV32_Encode_Immediate_12
        , ::testing::Range<vm::opcode::data_t>(0b0000'0000, 0b1'0000'0000'0000)
);

TEST_P(RV32_Encode_Immediate_12, SetTypeI)
{
    auto id = GetParam();
    auto value = Encoder::encode_i(id);
    Decoder parser{value};

    ASSERT_EQ(id, parser.decode_i_u());
}

TEST_P(RV32_Encode_Immediate_12, SetTypeS)
{
    auto id = GetParam();
    auto value = Encoder::encode_s(id);
    Decoder parser{value};

    ASSERT_EQ(id, parser.decode_s_u());
}

TEST_P(RV32_Encode_Immediate_12, SetTypeB)
{
    auto id = GetParam() << 1; // set id[0] == 0
    auto value = Encoder::encode_b(id);
    Decoder parser{value};

    ASSERT_EQ(id, parser.decode_b_u());
}

TEST_P(RV32_Encode_Immediate_12, SetTypeU)
{
    auto id = GetParam() << 12; // set id[11:0] == 0
    auto value = Encoder::encode_u(id);
    Decoder parser{value};

    ASSERT_EQ(id, parser.decode_u_u());
}

class RV32_Encode_Immediate_20: public ::testing::TestWithParam<vm::opcode::data_t> {};

INSTANTIATE_TEST_SUITE_P(
        ImmediateHight
        , RV32_Encode_Immediate_20
        , ::testing::Range<vm::opcode::data_t>(0b0, 0b1 << 10)
);

TEST_P(RV32_Encode_Immediate_20, SetTypeU)
{
    auto id_param = (GetParam() << 12); // set id[11:0] == 0

    {
        auto id = (id_param << 10) | id_param;
        auto value = Encoder::encode_u(id);
        Decoder parser{value};
        ASSERT_EQ(id, parser.decode_u_u());
    }
}
