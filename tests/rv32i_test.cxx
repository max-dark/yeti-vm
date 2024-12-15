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