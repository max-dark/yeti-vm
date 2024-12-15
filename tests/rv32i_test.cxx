#include <gtest/gtest.h>

#include <yeti-vm/vm_opcode.hxx>

using namespace vm::opcode;
using namespace std::literals;

TEST(InstructionParser, GetCode)
{
    for (opcode_t row = 0b00; row <= 0b11; ++row)
    {
        for (opcode_t col = 0b000; col <= 0b110; ++col)
        {
            opcode_t code = make_opcode(row, col);
            for (uint8_t i = 7; i < 32; ++i)
            {
                OpcodeBase parser{ .code = (1 << i) | code };
                EXPECT_EQ(parser.get_code(), code) << "fail";
            }
            auto id = get_op_id(std::bit_cast<OpcodeType>(code));
            EXPECT_NE(id, "UNKNOWN") << "should be not UNKNOWN";
        }

        {
            opcode_t code = make_opcode(row, 0b111);
            auto id = get_op_id(std::bit_cast<OpcodeType>(code));
            EXPECT_EQ(id, "UNKNOWN") << "should be UNKNOWN but id = " << id;
        }
    }
}

TEST(InstructionParser, GetRD)
{
    constexpr uint8_t reg_start = 7;
    constexpr uint8_t reg_size  = 5;
    constexpr uint8_t reg_last  = reg_start + reg_size;
    opcode_t mask = make_mask<reg_start, reg_size>();
    opcode_t lo = make_mask<0, reg_start - 2>();
    opcode_t hi = make_mask<0, reg_last  + 2>();
    OpcodeBase parser{};
    auto& code = parser.code;

    for (opcode_t i = lo; i <= hi; ++i)
    {
        code = i;
        auto expected = (i & mask) >> reg_start;
        ASSERT_EQ(parser.get_rd(), expected);
    }
}

TEST(InstructionParser, GetRS1)
{
    constexpr uint8_t reg_start = 15;
    constexpr uint8_t reg_size  =  5;
    constexpr uint8_t reg_last  =  reg_start + reg_size;
    constexpr opcode_t mask = make_mask<reg_start, reg_size>();
    // FIXME: range too big
    constexpr opcode_t lo = make_mask<0, reg_start - 2>();
    constexpr opcode_t hi = make_mask<0, reg_last  + 2>();
    OpcodeBase parser{};
    auto& code = parser.code;

    for (opcode_t i = lo; i <= hi; ++i)
    {
        code = i;
        auto expected = (i & mask) >> reg_start;
        ASSERT_EQ(parser.get_rs1(), expected);
    }
}

TEST(InstructionParser, GetRS2)
{
    constexpr uint8_t reg_start = 20;
    constexpr uint8_t reg_size  =  5;
    constexpr uint8_t reg_last  =  reg_start + reg_size;
    constexpr opcode_t mask = make_mask<reg_start, reg_size>();
    constexpr opcode_t lo = make_mask<0, reg_start - 2>();
    constexpr opcode_t hi = make_mask<0, reg_last  + 2>();
    OpcodeBase parser{};
    auto& code = parser.code;

    for (opcode_t i = lo; i <= hi; ++i)
    {
        code = i;
        auto expected = (i & mask) >> reg_start;
        ASSERT_EQ(parser.get_rs2(), expected);
    }
}

TEST(InstructionParser, GetA)
{
    constexpr uint8_t reg_start = 12;
    constexpr uint8_t reg_size  =  3;
    constexpr uint8_t reg_last  =  reg_start + reg_size;
    constexpr opcode_t mask = make_mask<reg_start, reg_size>();
    constexpr opcode_t lo = make_mask<0, reg_start - 2>();
    constexpr opcode_t hi = make_mask<0, reg_last  + 2>();
    OpcodeBase parser{};
    auto& code = parser.code;

    for (opcode_t i = lo; i <= hi; ++i)
    {
        code = i;
        auto expected = (i & mask) >> reg_start;
        ASSERT_EQ(parser.get_func3(), expected);
    }
}

TEST(InstructionParser, GetB)
{
    constexpr uint8_t reg_start = 25;
    constexpr uint8_t reg_size  =  7;
    //constexpr uint8_t reg_last  =  reg_start + reg_size;
    constexpr opcode_t mask = make_mask<reg_start, reg_size>();
    constexpr opcode_t lo = make_mask<0, reg_size + 2>();
    //constexpr opcode_t hi = lo << (reg_start - 2);
    OpcodeBase parser{};
    auto& code = parser.code;

    for (opcode_t i = 0; i <= lo; ++i)
    {
        code = i << (reg_start - 2);
        auto expected = (code & mask) >> reg_start;
        ASSERT_EQ(parser.get_func7(), expected);
    }
}
