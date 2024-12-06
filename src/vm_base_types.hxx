#pragma once

#include <stdlib.hxx>

namespace vm
{
/// type of program code container
using program_code_t = std::vector<std::uint8_t>;

/// register id
using register_no = std::uint8_t;

/// num of generic registers
constexpr register_no register_count = 32;
/// type of register value
using register_t = std::uint32_t;
/// common signed type
using signed_t = std::int32_t;
/// common unsigned type
using unsigned_t = std::uint32_t;

/// "register file"
using register_file = std::array<register_t, register_count + 1>; // generic + PC

/// register aliases
/// @see RISC-V assembly programmer's handbook, Chapter 20
enum RegAlias: register_no
{
    /// always zero
    zero = 0,
    /// return address
    ra = 1,
    /// stack pointer
    sp = 2,
    /// global pointer
    gp = 3,
    /// thread pointer
    tp = 4,
    /// temporary / alternative link register
    t0 = 5,
    /// temporary
    t1 = 6, t2 = 7,
    /// saved register / frame pointer
    s0 = 8, fp = 8,
    /// saved register
    s1 = 9,
    /// function arguments / return values
    a0 = 10, a1 = 11,
    /// function arguments
    a2 = 12, a3 = 13, a4 = 14, a5 = 15, a6 = 16, a7 = 17,
    /// saved registers
    s2 = 18, s3 = 19, s4 = 20, s5 = 21, s6 = 22, s7 = 23, s8 = 24, s9 = 25, s10 = 26, s11 = 27,
    /// temporaries
    t3 = 28, t4 = 29, t5 = 30, t6 = 31,
    /// program counter
    pc = register_count,
    /// unknown
    unknown
};

/**
 * get string representation of register alias
 * @param no register id
 * @return register alias
 */
std::string_view get_register_alias(register_no no);

} // namespace vm
