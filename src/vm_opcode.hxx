#pragma once
#include <vm_base_types.hxx>

namespace vm::opcode
{

using opcode_t = std::uint32_t;
using data_t = std::uint32_t;
using signed_t = std::int32_t;

enum BaseFormat: opcode_t
{
    UNKNOWN,
    R_TYPE,
    I_TYPE,
    S_TYPE,
    B_TYPE,
    U_TYPE,
    J_TYPE,
};

template<uint8_t start, uint8_t length>
consteval data_t make_mask()
{
    constexpr data_t all = ~data_t{0};
    constexpr data_t from_start = all << start;
    constexpr data_t from_end   = from_start << length;

    return from_start & (~from_end);
}
static_assert(make_mask<0, 8>() == 0b0'1111'1111, "wrong mask");
static_assert(make_mask<1, 8>() == 0b1'1111'1110, "wrong mask");

template<uint8_t start, uint8_t length>
constexpr data_t mask_value = make_mask<start, length>();

template<uint8_t start, uint8_t length>
consteval opcode::data_t get_bits(opcode::opcode_t code)
{
    constexpr uint8_t shift = (sizeof(code) * 8) - length;
    constexpr opcode::opcode_t mask = (~static_cast<opcode::opcode_t>(0)) >> shift;

    return (code >> start) & mask;
}
static_assert(get_bits<0, 1>(0b0000'0001) == opcode_t{0b0000'0001}, "something wrong");
static_assert(get_bits<0, 2>(0b0000'0011) == opcode_t{0b0000'0011}, "something wrong");
static_assert(get_bits<1, 1>(0b0000'0010) == opcode_t{0b0000'0001}, "something wrong");

constexpr data_t get_bits(opcode_t code, uint8_t start, uint8_t length)
{
    uint8_t shift = (sizeof(code) * 8) - length;
    opcode_t mask = static_cast<opcode_t>(-1) >> shift;

    return (code >> start) & mask;
}

template<uint8_t start, uint8_t length>
constexpr data_t shift_bits(opcode_t code, uint8_t to)
{
    constexpr auto mask = mask_value<start, length>;
    return ((code & mask) >> start) << to;
}

template<uint8_t start, uint8_t to, uint8_t length>
constexpr data_t shift_bits(opcode_t code)
{
    constexpr auto mask = mask_value<start, length>;
    return ((code & mask) >> start) << to;
}
static_assert(shift_bits<0, 1, 1>(0b0000'0001) == 0b0000'0010);
static_assert(shift_bits<0, 1, 2>(0b0000'0011) == 0b0000'0110);
static_assert(shift_bits<0, 2, 1>(0b0000'0001) == 0b0000'0100);
static_assert(shift_bits<0, 2, 2>(0b0000'0011) == 0b0000'1100);

inline std::string to_hex(data_t num)
{
    return std::format("{:08x}", num);
}

data_t extend_sign(data_t value, opcode_t code);

struct OpcodeBase
{
    opcode_t code;

    [[nodiscard]]
    opcode_t get_code() const
    {
        return get_bits(code, 0, 7);
    }

    [[nodiscard]]
    register_no get_rs2() const
    {
        return get_bits(code, 20, 5);
    }

    [[nodiscard]]
    register_no get_rs1() const
    {
        return get_bits(code, 15, 5);
    }

    [[nodiscard]]
    register_no get_rd() const
    {
        return get_bits(code, 7, 5);
    }

    [[nodiscard]]
    data_t get_func3() const
    {
        return get_bits(code, 12, 3);
    }

    [[nodiscard]]
    data_t get_func7() const
    {
        return get_bits(code, 25, 7);
    }

    [[nodiscard]]
    data_t decode_i() const;

    [[nodiscard]]
    data_t decode_s() const;

    [[nodiscard]]
    data_t decode_b() const;

    [[nodiscard]]
    data_t decode_u() const;

    [[nodiscard]]
    data_t decode_j() const;
};
static_assert(sizeof(OpcodeBase) == sizeof(opcode_t));

/// @see Instruction Length Encoding
size_t op_size(opcode_t code);

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
    LOAD   = make_opcode(0b00, 0b000), // read value from memory
    STORE  = make_opcode(0b01, 0b000), // write value to memory
    MADD   = make_opcode(0b10, 0b000), // "Multiple and Add"(float/double)
    BRANCH = make_opcode(0b11, 0b000), // conditional jump

    LOAD_FP  = make_opcode(0b00, 0b001), // read value from memory(float/double)
    STORE_FP = make_opcode(0b01, 0b001), // write value to memory(float/double)
    MSUB     = make_opcode(0b10, 0b001), // "Multiple and Sub"(float/double)
    JALR     = make_opcode(0b11, 0b001), // jump to offset_t from register

    CUSTOM_0 = make_opcode(0b00, 0b010), // for extensions
    CUSTOM_1 = make_opcode(0b01, 0b010), // for extensions
    NMSUB    = make_opcode(0b10, 0b010), // "Multiple and Sub"(float/double)
    // R_11_010 = make_opcode(0b11, 0b010), // reserved

    MISC_MEM = make_opcode(0b00, 0b011), // sync, barriers, etc
    AMO      = make_opcode(0b01, 0b011), // atomic ops
    NMADD    = make_opcode(0b10, 0b011), // "Multiple and Add"(float/double)
    JAL      = make_opcode(0b11, 0b011), // jump to offset_t

    OP_IMM = make_opcode(0b00, 0b100), // op with const
    OP     = make_opcode(0b01, 0b100), // op with registers
    OP_FP  = make_opcode(0b10, 0b100), // op with registers(float/double)
    SYSTEM = make_opcode(0b11, 0b100), // CSR(Control and Status Registers), counters, debug, SysCals

    AUIPC = make_opcode(0b00, 0b101), // "Add upper immediate to PC"
    LUI   = make_opcode(0b01, 0b101), // "Load upper immediate"
    // R_10_101 = make_opcode(0b01, 0b101), // reserved
    // R_11_101 = make_opcode(0b11, 0b101), // reserved

    OP_IMM_32 = make_opcode(0b00, 0b110), // only for 64bit
    OP_32     = make_opcode(0b01, 0b110), // only for 64bit
    CUSTOM_2  = make_opcode(0b10, 0b110), // for extensions
    CUSTOM_3  = make_opcode(0b11, 0b110), // for extensions
};

std::string_view get_op_id(OpcodeType code);

bool have_ext_a(opcode_t code);

bool have_ext_b(opcode_t code);

} //namespace vm::opcode
