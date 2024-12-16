#pragma once

#include <cstdint>
#include <concepts>
#include <type_traits>
#include <bit>

namespace vm::bit_tools
{
static_assert(std::endian::native == std::endian::little, "endianness restriction: only little-endian architectures are supported");

template<std::integral Number>
struct bits
{
    using value_type = Number;
    using signed_type = std::make_signed_t<value_type>;
    using unsigned_type = std::make_unsigned_t<value_type>;
    using offset_type = std::uint8_t;

    /// value with all bits
    static constexpr value_type all_bits = ~value_type{};
    static constexpr unsigned_type all_bits_mask = ~unsigned_type{};

    /// type size in bits
    static constexpr offset_type bit_count = sizeof(value_type) * 8;
    /// least significant bit position
    static constexpr offset_type lsb_pos = 0;
    /// most significant bit position
    static constexpr offset_type msb_pos = bit_count - 1;
    /// sign bit position
    static constexpr offset_type sign_pos = msb_pos;
    /// mask for sign bit
    static constexpr unsigned_type sign_mask = unsigned_type{1} << sign_pos;

    /// value of make_mask
    template<offset_type start, offset_type length>
    static constexpr unsigned_type mask_value = make_mask<start, length>();

    /// convert value to unsigned
    static constexpr unsigned_type to_unsigned(value_type value)
    {
        return std::bit_cast<unsigned_type>(value);
    }

    /// convert value to signed
    static constexpr signed_type to_signed(value_type value)
    {
        return std::bit_cast<signed_type>(value);
    }

    /**
     * create bit mask
     * @tparam start from
     * @tparam length bit count
     * @return mask value
     */
    template<offset_type start, offset_type length>
    static consteval value_type make_mask()
    {
        constexpr unsigned_type from_start = (all_bits_mask << start) & all_bits_mask;
        constexpr unsigned_type from_end   = (from_start << length) & all_bits_mask;

        return from_start & (~from_end);
    }

    /// get bit field
    template<offset_type start, offset_type length>
    static constexpr value_type get_bits(value_type code)
    {
        constexpr unsigned_type mask = make_mask<0, length>();

        return (code >> start) & mask;
    }

    /// get bit field
    static constexpr value_type get_bits(value_type code, offset_type start, offset_type length)
    {
        offset_type shift = bit_count - length;
        unsigned_type mask = all_bits_mask >> shift;

        return (code >> start) & mask;
    }

    /// get bit field
    template<offset_type LSB, offset_type MSB>
    static constexpr inline value_type get_range(value_type code)
    {
        static_assert(MSB >= LSB, "invalid range");
        constexpr offset_type shift = bit_count - (MSB - LSB + 1);
        constexpr unsigned_type mask = all_bits_mask >> shift;

        return (code >> LSB) & mask;
    }

    /// get single bit by index
    template<unsigned Index>
    static constexpr inline value_type get_range(value_type code)
    {
        return get_range<Index, Index>(code);
    }

    /// shift bit field
    template<offset_type start, offset_type length>
    static constexpr value_type shift_bits(value_type code, offset_type to)
    {
        constexpr unsigned_type mask = make_mask<start, length>();
        return ((code & mask) >> start) << to;
    }

    /// shift bit field
    template<offset_type start, offset_type to, offset_type length>
    static constexpr value_type shift_bits(value_type code)
    {
        constexpr unsigned_type mask = make_mask<start, length>();
        return ((code & mask) >> start) << to;
    }

    /**
     * fill bits from position to msb_pos with bit on code[position]
     * @tparam position position of bit to copy
     * @param code value to extend
     * @return sign-extended value
     */
    template<offset_type position>
    static constexpr value_type extend_sign(value_type code)
    {
        signed_type sign = (code >> position) << sign_pos;
        signed_type mask = sign >> (sign_pos - position);
        return code | mask;
    }

    /**
     * fill bits from position to msb_pos with bit on code[position]
     * @param code value to extend
     * @param position position of bit to copy
     * @return sign-extended value
     */
    static constexpr value_type extend_sign(value_type code, offset_type position)
    {
        signed_type sign = (code >> position) << sign_pos;
        signed_type mask = sign >> (sign_pos - position);
        return code | mask;
    }
    /**
     * find position first bit != 0 and bits from this position to msb_pos with ones
     * @param code value to extend
     * @return sign-extended value
     */
    static constexpr value_type extend_sign(value_type code)
    {
        offset_type position = bit_count - std::countl_zero(code) - 1;
        if (position >= bit_count) return code;
        signed_type sign = (code >> position) << sign_pos;
        signed_type mask = sign >> (sign_pos - position);
        return code | mask;
    }
};

struct bits_u8 final: bits<std::uint8_t> {};
struct bits_i8 final: bits<std::int8_t > {};

struct bits_u16 final: bits<std::uint16_t> {};
struct bits_i16 final: bits<std::int16_t > {};

struct bits_u32 final: bits<std::uint32_t> {};
struct bits_i32 final: bits<std::int32_t > {};

struct bits_u64 final: bits<std::uint64_t> {};
struct bits_i64 final: bits<std::int64_t > {};

} // namespace vm::bit_tools
