#pragma once

#include <cstdint>
#include <concepts>
#include <bit>

namespace vm::bit_tools
{

template<std::integral Number>
struct bits
{
    using value_type = Number;
    using offset_type = std::uint8_t;

    /// value with all bits
    static constexpr value_type all_bits = ~value_type{};

    /// value of make_mask
    template<offset_type start, offset_type length>
    static constexpr value_type mask_value = make_mask<start, length>();

    /**
     * create bit mask
     * @tparam start from
     * @tparam length bit count
     * @return mask value
     */
    template<offset_type start, offset_type length>
    static consteval value_type make_mask()
    {
        constexpr value_type from_start = all_bits << start;
        constexpr value_type from_end   = from_start << length;

        return from_start & (~from_end);
    }
    /// get bit field
    template<offset_type start, offset_type length>
    static constexpr value_type get_bits(value_type code)
    {
        constexpr offset_type shift = (sizeof(code) * 8) - length;
        constexpr value_type mask = all_bits >> shift;

        return (code >> start) & mask;
    }

    /// get bit field
    static constexpr value_type get_bits(value_type code, offset_type start, offset_type length)
    {
        offset_type shift = (sizeof(code) * 8) - length;
        value_type mask = all_bits >> shift;

        return (code >> start) & mask;
    }

    /// get bit field
    template<offset_type LSB, offset_type MSB>
    static constexpr inline value_type get_range(value_type code)
    {
        static_assert(MSB >= LSB, "invalid range");
        constexpr offset_type shift = (sizeof(code) * 8) - (MSB - LSB + 1);
        constexpr value_type mask = all_bits >> shift;

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
        constexpr value_type mask = make_mask<start, length>();
        return ((code & mask) >> start) << to;
    }

    /// shift bit field
    template<offset_type start, offset_type to, offset_type length>
    static constexpr value_type shift_bits(value_type code)
    {
        constexpr value_type mask = make_mask<start, length>();
        return ((code & mask) >> start) << to;
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
