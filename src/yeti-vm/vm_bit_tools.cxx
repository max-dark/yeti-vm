#include "vm_bit_tools.hxx"

namespace vm::bit_tools
{

static_assert(bits_u32::make_mask<0, 8>() == 0b0'1111'1111, "wrong mask");
static_assert(bits_u32::make_mask<1, 8>() == 0b1'1111'1110, "wrong mask");
static_assert(bits_i32::make_mask<0, 8>() == 0b0'1111'1111, "wrong mask");
static_assert(bits_i32::make_mask<1, 8>() == 0b1'1111'1110, "wrong mask");

static_assert(bits_u32::shift_bits<0, 1, 1>(0b0000'0001) == 0b0000'0010);
static_assert(bits_u32::shift_bits<0, 1, 2>(0b0000'0011) == 0b0000'0110);
static_assert(bits_u32::shift_bits<0, 2, 1>(0b0000'0001) == 0b0000'0100);
static_assert(bits_u32::shift_bits<0, 2, 2>(0b0000'0011) == 0b0000'1100);

namespace // test for signed version
{
    constexpr bits_i8::value_type value = -1;
    template<bits_i8::offset_type pos>
    constexpr bits_i8::value_type expected = 0b0000'0001 << pos;
    static_assert(bits_i8::get_bits<0, 1>(value) == expected<0>);
    static_assert(bits_i8::get_bits<1, 1>(value) == expected<0>);
}

static_assert(bits_u32::get_bits<0, 1>(0b0000'0001) == bits_u32::value_type{0b0000'0001}, "something wrong");
static_assert(bits_u32::get_bits<0, 2>(0b0000'0011) == bits_u32::value_type{0b0000'0011}, "something wrong");
static_assert(bits_u32::get_bits<1, 1>(0b0000'0010) == bits_u32::value_type{0b0000'0001}, "something wrong");

static_assert(bits_u32::get_bits< 0, 7>(0x02c58533) == bits_u32::value_type{0b0011'0011}, "something wrong");
static_assert(bits_u32::get_bits<12, 3>(0x02c58533) == bits_u32::value_type{0b0000'0000}, "something wrong");
static_assert(bits_u32::get_bits<12, 3>(0x02c5c533) == bits_u32::value_type{0b0000'0100}, "something wrong");
static_assert(bits_u32::get_bits<12, 3>(0x02c5f533) == bits_u32::value_type{0b0000'0111}, "something wrong");

static_assert(bits_u32::to_signed(bits_u32::all_bits) == bits_i32::all_bits);
static_assert(bits_i32::to_unsigned(bits_i32::all_bits) == bits_u32::all_bits);

static_assert(bits_u32::extend_sign<3>(0) == 0);
static_assert(bits_u32::extend_sign<3>(1) == 1);

static_assert(bits_u16::extend_sign<3>(0b01000) == bits_u16::value_type(~0b0111u));
static_assert(bits_u32::extend_sign<2>(0b00100) == bits_u32::value_type(~0b0011u));

static_assert(std::countl_zero<std::uint8_t>(0b0000'0000) == 8);
static_assert(std::countl_zero<std::uint8_t>(0b0001'0000) == 3);

static_assert(bits_u8::extend_sign(0b01000) == bits_u8::value_type(~0b0111u));
static_assert(bits_u8::extend_sign(0b00100) == bits_u8::value_type(~0b0011u));

static_assert(bits_u8::extend_sign(0b0000) == bits_u8::value_type(0));
static_assert(bits_u8::extend_sign(bits_u8::sign_mask) == bits_u8::sign_mask);
static_assert(bits_u8::extend_sign(~0) == bits_u8::value_type(~0));

} // namespace vm::bit_tools
