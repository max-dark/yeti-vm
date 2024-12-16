#pragma once
#include "vm_base_types.hxx"
#include "vm_bit_tools.hxx"

namespace vm::opcode
{

/// base type for instruction
using opcode_t = std::uint32_t;
/// unsigned data type
using data_t = std::uint32_t;
/// signed data type
using signed_t = std::int32_t;

/// instruction encoding format
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

/**
 * create bit mask
 * @tparam start from
 * @tparam length bit count
 * @return mask value
 */
template<uint8_t start, uint8_t length>
consteval data_t make_mask()
{
    return bit_tools::bits<data_t>::make_mask<start, length>();
}
static_assert(make_mask<0, 8>() == 0b0'1111'1111, "wrong mask");
static_assert(make_mask<1, 8>() == 0b1'1111'1110, "wrong mask");

/// value of make_mask
template<uint8_t start, uint8_t length>
constexpr data_t mask_value = bit_tools::bits<data_t>::make_mask<start, length>();

/// get bit field
template<uint8_t start, uint8_t length>
consteval opcode::data_t get_bits(opcode::opcode_t code)
{
    return bit_tools::bits<data_t>::get_bits<start, length>(code);
}
/// get bit field
constexpr data_t get_bits(opcode_t code, uint8_t start, uint8_t length)
{
    return bit_tools::bits<data_t>::get_bits(code, start, length);
}

/// shift bit field
template<uint8_t start, uint8_t length>
constexpr data_t shift_bits(opcode_t code, uint8_t to)
{
    return bit_tools::bits<data_t>::shift_bits<start, length>(code, to);
}

/// shift bit field
template<uint8_t start, uint8_t to, uint8_t length>
constexpr data_t shift_bits(opcode_t code)
{
    return bit_tools::bits<data_t>::shift_bits<start, to, length>(code);
}
/**
 * convert num to hex string
 * @param num
 * @return hex string
 */
inline std::string to_hex(data_t num)
{
    return std::format("{:08x}", num);
}

/**
 * fill sign by MSB of code
 * @param value value to extend
 * @param code MSB
 * @return extended value
 */
data_t extend_sign(data_t value, opcode_t code);

/// opcode decoding utility
struct OpcodeBase
        : protected bit_tools::bits<opcode_t>
{
    /// instruction code
    opcode_t code;

    OpcodeBase() = default;
    explicit OpcodeBase(opcode_t code): code{code} {}

    /// opcode group ID
    [[nodiscard]]
    opcode_t get_code() const
    {
        return get_bits(code, 0, 7);
    }

    /// get rs2(rhs) register ID
    [[nodiscard]]
    register_no get_rs2() const
    {
        return get_bits(code, 20, 5);
    }

    /// get rs1(lhs) register ID
    [[nodiscard]]
    register_no get_rs1() const
    {
        return get_bits(code, 15, 5);
    }

    /// get rd(dest) register ID
    [[nodiscard]]
    register_no get_rd() const
    {
        return get_bits(code, 7, 5);
    }

    /// get "func A" ID
    [[nodiscard]]
    data_t get_func3() const
    {
        return get_bits(code, 12, 3);
    }

    /// get "func B" ID
    [[nodiscard]]
    data_t get_func7() const
    {
        return get_bits(code, 25, 7);
    }

    /// decode immediate / I-type / sign extended
    [[nodiscard]]
    data_t decode_i() const;

    /// decode immediate / I-type / zero extended
    [[nodiscard]]
    data_t decode_i_u() const;

    /// decode immediate / S-type / sign extended
    [[nodiscard]]
    data_t decode_s() const;

    /// decode immediate / S-type / zero extended
    [[nodiscard]]
    data_t decode_s_u() const;

    /// decode immediate / B-type / sign extended
    [[nodiscard]]
    data_t decode_b() const;

    /// decode immediate / B-type / zero extended
    [[nodiscard]]
    data_t decode_b_u() const;

    /// decode immediate / U-type
    [[nodiscard]]
    data_t decode_u() const;
    [[nodiscard]]
    data_t decode_u_u() const { return decode_u(); }

    /// decode immediate / J-type / sign extended
    [[nodiscard]]
    data_t decode_j() const;

    /// decode immediate / J-type / zero extended
    [[nodiscard]]
    data_t decode_j_u() const;
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

/// opcode group ID's
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
    R_11_010 = make_opcode(0b11, 0b010), // reserved

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
    R_10_101 = make_opcode(0b10, 0b101), // reserved
    R_11_101 = make_opcode(0b11, 0b101), // reserved

    OP_IMM_32 = make_opcode(0b00, 0b110), // only for 64bit
    OP_32     = make_opcode(0b01, 0b110), // only for 64bit
    CUSTOM_2  = make_opcode(0b10, 0b110), // for extensions
    CUSTOM_3  = make_opcode(0b11, 0b110), // for extensions
};

/// get mnemonic of opcode group ID
std::string_view get_op_id(OpcodeType code);

/// get mnemonic of opcode group ID
inline std::string_view get_code_id(opcode_t code) {
    return get_op_id(static_cast<OpcodeType>(code & mask_value<0, 7>));
}

} //namespace vm::opcode
