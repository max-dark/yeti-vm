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

constexpr opcode_t sign_mask = make_mask<31, 1>();

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

// test for "S-type immediate"
TEST(ImmediateParser, DecodeTypeS)
{
    constexpr opcode_t range = make_mask<0, 12>();

    for (opcode_t i = 0; i <= range; ++i)
    {
        auto a = shift_bits<11, 31, 1>(i);
        auto b = shift_bits< 5, 25, 6>(i);
        auto c = shift_bits< 1,  8, 4>(i);
        auto d = shift_bits< 0,  7, 1>(i);

        auto encoded = (a | b | c | d);
        auto sign = encoded & sign_mask;
        OpcodeBase parser{.code = encoded};
        auto value = parser.decode_s();

        ASSERT_EQ(value & range, i) << std::hex << std::showbase << "wrong value for i = " << i;
        ASSERT_EQ(value & sign_mask, sign)  << std::hex << std::showbase << "wrong sign for i = " << i;
        auto extend = sign ? ~range : 0;
        ASSERT_EQ(value & extend, extend)  << std::hex << std::showbase << "wrong extend for i = " << i;
    }
}

// test for "B-type immediate"
TEST(ImmediateParser, DecodeTypeB)
{
    constexpr opcode_t range = make_mask<0, 13>();
    constexpr opcode_t values = make_mask<1, 12>();
    constexpr opcode_t v_mask = make_mask<0, 13>();

    for (opcode_t i = 0; i <= range; i += 2) // only even, v[0] == 0
    {
        ASSERT_EQ(i & 1, 0) << "wrong test range: v[0] != 0";
        auto a = shift_bits<12, 31, 1>(i);
        auto c = shift_bits<11,  7, 1>(i);
        auto b = shift_bits< 5, 25, 6>(i);
        auto d = shift_bits< 1,  8, 4>(i);

        auto encoded = (a | b | c | d | 0);
        auto sign = encoded & sign_mask;
        OpcodeBase parser{.code = encoded};
        auto value = parser.decode_b();

        ASSERT_EQ(value & values, i) << std::hex << std::showbase << "wrong value for i = " << i;
        ASSERT_EQ(value & sign_mask, sign)  << std::hex << std::showbase << "wrong sign for i = " << i;
        auto extend = sign ? ~v_mask : 0;
        ASSERT_EQ(value & extend, extend)  << std::hex << std::showbase << "wrong extend for i = " << i;
    }
}

// test for "U-type immediate"
TEST(ImmediateParser, DecodeTypeU)
{
    constexpr opcode_t range  = make_mask< 0, 20>();
    constexpr opcode_t values = make_mask<12, 20>();
    constexpr opcode_t v_mask = ~0u; // make_mask<0, 32>();

    for (opcode_t i = 0; i <= range; ++i)
    {
        auto encoded = i << 12;
        auto sign = encoded & sign_mask;
        OpcodeBase parser{.code = encoded};
        auto value = parser.decode_u();

        ASSERT_EQ(value & values, encoded) << std::hex << std::showbase << "wrong value for i = " << i;
        ASSERT_EQ(value & sign_mask, sign)  << std::hex << std::showbase << "wrong sign for i = " << i;
        auto extend = sign ? ~v_mask : 0;
        ASSERT_EQ(value & extend, extend)  << std::hex << std::showbase << "wrong extend for i = " << i;
    }
}
