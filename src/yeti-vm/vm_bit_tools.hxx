#pragma once

namespace vm::bit_tools
{

template<typename Number>
struct bits
{
    using value_t = Number;

    /// get bit field
    template<unsigned LSB, unsigned MSB>
    static constexpr inline value_t get_range(value_t code)
    {
        static_assert(MSB >= LSB, "invalid range");
        constexpr unsigned shift = (sizeof(code) * 8) - (MSB - LSB + 1);
        constexpr value_t mask = (~static_cast<value_t>(0)) >> shift;

        return (code >> LSB) & mask;
    }

    /// get single bit by index
    template<unsigned Index>
    static constexpr inline value_t get_range(value_t code)
    {
        return get_range<Index, Index>(code);
    }

};

} // namespace vm::bit_tools
