#include <gtest/gtest.h>

#include <yeti-vm/vm_opcode.hxx>

using namespace vm::opcode;
using namespace std::literals;

#include <limits>
namespace remove_me_later
{
TEST(Shift, ShiftRightArithmetic)
{
    int i = ~0; // == -1
    int m = ~std::numeric_limits<int>::max();
    int n = ~m;
    EXPECT_EQ(i, -1);
    EXPECT_EQ(i >> 1, i);
    EXPECT_EQ(n & m, 0);
    EXPECT_EQ(m >> 1, 0b11 << 30);
}

} // namespace remove_me_later

// test for "I-type immediate"
TEST(ImmediateParser, DecodeTypeI)
{
    constexpr uint8_t start = 20;
    constexpr uint8_t size  = 12;
    constexpr opcode_t mask = make_mask<start, size>();
    constexpr opcode_t range = make_mask<0, size>();
    constexpr opcode_t sign = make_mask<31, 1>();
    constexpr opcode_t extend = make_mask<size, 20>();

    for (opcode_t i = 0; i <= range; ++i)
    {
        const OpcodeBase parser{.code = i << start};

        auto u_value = parser.decode_i_u();
        ASSERT_EQ(u_value, i) << std::hex << std::showbase << "wrong zero extend for i = " << i;

        auto s_value = parser.decode_i();
        opcode_t e_expected = ((i << start) & sign) ? extend : 0;
        opcode_t e_actual = s_value & e_expected;
        opcode_t v_expected = i | e_expected;
        ASSERT_EQ(e_actual, e_expected) << std::hex << std::showbase << "wrong sign extend for i = " << i;
        ASSERT_EQ(s_value, v_expected) << std::hex << std::showbase << "wrong s_value for i = " << i;
    }
}