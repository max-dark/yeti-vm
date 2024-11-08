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
        bf data : 25;

        bf value() const
        {
            return 0;
        }
    };
    static_assert(sizeof(base_type) == sizeof(opcode_t));
    struct r_type
    {
        bf op : 7;
        bf rd : 5;
        bf f3 : 3;
        bf rs1 : 5;
        bf rs2 : 5;
        bf f7 : 7;

        bf value() const
        {
            return 0;
        }
    };
    static_assert(sizeof(r_type) == sizeof(opcode_t));

    struct i_type
    {
        bf op : 7;
        bf rd : 5;
        bf f3 : 3;
        bf rs1 : 5;
        bf i_11_0 : 12;

        bf value() const
        {
            return i_11_0;
        }
    };
    static_assert(sizeof(i_type) == sizeof(opcode_t));

    struct s_type
    {
        bf op : 7;
        bf i_4_0 : 5;
        bf f3 : 3;
        bf rs1 : 5;
        bf rs2 : 5;
        bf i_11_5 : 7;

        bf value() const
        {
            return (i_11_5 << 5) | i_4_0;
        }
    };
    static_assert(sizeof(s_type) == sizeof(opcode_t));

    struct b_type
    {
        bf op : 7;
        bf i_11 : 1;
        bf i_4_1 : 4;
        bf f3 : 3;
        bf rs1 : 5;
        bf rs2 : 5;
        bf i_10_5 : 6;
        bf i_12 : 1;

        bf value() const
        {
            return (i_12 << 12) | (i_11 << 11) | (i_10_5 << 5) | (i_4_1 << 1) | 0; // OoO
        }
    };
    static_assert(sizeof(b_type) == sizeof(opcode_t));

    struct u_type
    {
        bf op : 7;
        bf rd : 5;
        bf i_31_12 : 20;

        bf value() const
        {
            return (i_31_12 << 12) | 0b0000'0000'0000;
        }
    };
    static_assert(sizeof(u_type) == sizeof(opcode_t));

    struct j_type
    {
        bf op : 7;
        bf rd : 5;
        bf i_19_12 : 8;
        bf i_11 : 1;
        bf i_10_1 : 10;
        bf i_20 : 1;

        bf value() const
        {
            return (i_20 << 20) | (i_19_12 << 12) | (i_11 << 11) | (i_10_1 << 1) | 0;
        }
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


/// @see Instruction Length Encoding
size_t op_size(opcode_t code)
{
    if ((0b00000011 & code) != 0b0000011) return 1;
    if ((0b00011100 & code) != 0b0011100) return 2;
    if ((0b00111111 & code) == 0b0011111) return 3;
    if ((0b01111111 & code) == 0b0111111) return 3;

    return 0;
}

/// @see RV32/64G Instruction Set Listings
constexpr opcode_t make_opcode(opcode_t i65, opcode_t i42)
{
    return (i65 << 5) | (i42 << 2) | 0b011;
}
static_assert(make_opcode(0b11, 0b111) == 0b011'111'11);
static_assert(make_opcode(0b11, 0b110) == 0b011'110'11);
static_assert(make_opcode(0b10, 0b111) == 0b010'111'11);
static_assert(make_opcode(0b10, 0b110) == 0b010'110'11);

enum OpcodeType: opcode_t
{
    LOAD   = make_opcode(0b00, 0b000),
    STORE  = make_opcode(0b01, 0b000),
    MADD   = make_opcode(0b10, 0b000),
    BRANCH = make_opcode(0b11, 0b000),

    LOAD_FP  = make_opcode(0b00, 0b001),
    STORE_FP = make_opcode(0b01, 0b001),
    MSUB     = make_opcode(0b10, 0b001),
    JALR     = make_opcode(0b11, 0b001),

    CUSTOM_0 = make_opcode(0b00, 0b010),
    CUSTOM_1 = make_opcode(0b01, 0b010),
    NMSUB = make_opcode(0b01, 0b010),
    // R_11_010 = make_opcode(0b11, 0b010), // reserved

    MISC_MEM = make_opcode(0b00, 0b011),
    AMO = make_opcode(0b01, 0b011),
    NMADD = make_opcode(0b01, 0b011),
    JAL = make_opcode(0b11, 0b011),

    OP_IMM = make_opcode(0b00, 0b100),
    OP = make_opcode(0b01, 0b100),
    OP_FP = make_opcode(0b01, 0b100),
    SYSTEM = make_opcode(0b11, 0b100),

    AUIPC = make_opcode(0b00, 0b101),
    LUI = make_opcode(0b01, 0b101),
    // R_10_101 = make_opcode(0b01, 0b101), // reserved
    // R_11_101 = make_opcode(0b11, 0b101), // reserved

    OP_IMM_32 = make_opcode(0b00, 0b011),
    OP_32 = make_opcode(0b01, 0b011),
    CUSTOM_2 = make_opcode(0b01, 0b011),
    CUSTOM_3 = make_opcode(0b11, 0b011),
};

} // namespace opcode
} // namespace vm

int main(int argc, char** argv)
{
    return 0;
}