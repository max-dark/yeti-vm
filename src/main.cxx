#include <iostream>
#include "stdlib.hxx"

namespace fs = std::filesystem;

using program = std::vector<std::uint8_t>;

std::optional<program> load_program(const fs::path& programFile)
{
    program data;

    return std::nullopt;
}

namespace vm
{
using register_no = std::uint8_t;
constexpr register_no register_count = 32;
using register_t = std::uint32_t;
using registers = std::array<register_t, register_count + 1>; // generic + PC

enum RegAlias: register_no
{
    zero = 0,
    pc = register_count
};

namespace opcode
{
using opcode_t = std::uint32_t;
using function_t = std::uint32_t;
using data_t = std::uint32_t;

struct OpcodeBase
{
    using bf = opcode_t;

    struct base_type
    {
        bf op : 7;
        bf d0 : 25;
    };
    static_assert(sizeof(base_type) == sizeof(opcode_t));
    struct r_type
    {
        bf op : 7;
        bf rd : 5;
        bf f3 : 3;
        bf r1 : 5;
        bf r2 : 5;
        bf f7 : 7;
    };
    static_assert(sizeof(r_type) == sizeof(opcode_t));

    struct i_type
    {
        bf op : 7;
        bf rd : 5;
        bf f3 : 3;
        bf r1 : 5;
        bf i0 : 12;
    };
    static_assert(sizeof(i_type) == sizeof(opcode_t));

    struct s_type
    {
        bf op : 7;
        bf i0 : 5;
        bf f3 : 3;
        bf r1 : 5;
        bf r2 : 5;
        bf i1 : 7;
    };
    static_assert(sizeof(s_type) == sizeof(opcode_t));

    struct b_type
    {
        bf op : 7;
        bf i0 : 5;
        bf f3 : 3;
        bf r1 : 5;
        bf r2 : 5;
        bf i1 : 7;
    };
    static_assert(sizeof(b_type) == sizeof(opcode_t));

    struct u_type
    {
        bf op : 7;
        bf rd : 5;
        bf i0 : 20;
    };
    static_assert(sizeof(u_type) == sizeof(opcode_t));

    struct j_type
    {
        bf op : 7;
        bf rd : 5;
        bf i0 : 20;
    };
    static_assert(sizeof(j_type) == sizeof(opcode_t));

    union
    {
        opcode_t code;
        base_type base;
        r_type r;
        i_type i;
        s_type s;
        b_type b;
        u_type u;
        j_type j;
    };
};
static_assert(sizeof(OpcodeBase) == sizeof(opcode_t));

constexpr opcode_t make_opcode(opcode_t i65, opcode_t i42)
{
    return (i65 << 5) | (i42 << 2) | 0b011;
}
static_assert(make_opcode(0b11, 0b111) == 0b011'111'11);
static_assert(make_opcode(0b11, 0b110) == 0b011'110'11);
static_assert(make_opcode(0b10, 0b111) == 0b010'111'11);
static_assert(make_opcode(0b10, 0b110) == 0b010'110'11);

} // namespace opcode
} // namespace vm

int main(int argc, char** argv)
{
    return 0;
}