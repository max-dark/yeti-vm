#include "vm_bit_tools.hxx"

namespace vm::bit_tools
{

static_assert(bits_u32::shift_bits<0, 1, 1>(0b0000'0001) == 0b0000'0010);
static_assert(bits_u32::shift_bits<0, 1, 2>(0b0000'0011) == 0b0000'0110);
static_assert(bits_u32::shift_bits<0, 2, 1>(0b0000'0001) == 0b0000'0100);
static_assert(bits_u32::shift_bits<0, 2, 2>(0b0000'0011) == 0b0000'1100);

static_assert(bits_u32::get_bits<0, 1>(0b0000'0001) == bits_u32::value_type{0b0000'0001}, "something wrong");
static_assert(bits_u32::get_bits<0, 2>(0b0000'0011) == bits_u32::value_type{0b0000'0011}, "something wrong");
static_assert(bits_u32::get_bits<1, 1>(0b0000'0010) == bits_u32::value_type{0b0000'0001}, "something wrong");

static_assert(bits_u32::get_bits< 0, 7>(0x02c58533) == bits_u32::value_type{0b0011'0011}, "something wrong");
static_assert(bits_u32::get_bits<12, 3>(0x02c58533) == bits_u32::value_type{0b0000'0000}, "something wrong");
static_assert(bits_u32::get_bits<12, 3>(0x02c5c533) == bits_u32::value_type{0b0000'0100}, "something wrong");
static_assert(bits_u32::get_bits<12, 3>(0x02c5f533) == bits_u32::value_type{0b0000'0111}, "something wrong");

static_assert(bits_u32::to_signed(bits_u32::all_bits) == bits_i32::all_bits);
static_assert(bits_i32::to_unsigned(bits_i32::all_bits) == bits_u32::all_bits);

} // namespace vm::bit_tools
