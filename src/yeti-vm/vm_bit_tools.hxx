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
    static consteval value_type get_bits(value_type code)
    {
        constexpr offset_type shift = (sizeof(code) * 8) - length;
        constexpr value_type mask = all_bits >> shift;

        return (code >> start) & mask;
    }

    /// get bit field
    static constexpr value_type get_bits(value_type code, uint8_t start, uint8_t length)
    {
        uint8_t shift = (sizeof(code) * 8) - length;
        value_type mask = all_bits >> shift;

        return (code >> start) & mask;
    }

    /// get bit field
    template<offset_type LSB, offset_type MSB>
    static constexpr inline value_type get_range(value_type code)
    {
        static_assert(MSB >= LSB, "invalid range");
        constexpr unsigned shift = (sizeof(code) * 8) - (MSB - LSB + 1);
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
    template<uint8_t start, uint8_t length>
    static constexpr value_type shift_bits(value_type code, uint8_t to)
    {
        constexpr value_type mask = make_mask<start, length>();
        return ((code & mask) >> start) << to;
    }

    /// shift bit field
    template<uint8_t start, uint8_t to, uint8_t length>
    static constexpr value_type shift_bits(value_type code)
    {
        constexpr value_type mask = make_mask<start, length>();
        return ((code & mask) >> start) << to;
    }
};

} // namespace vm::bit_tools
