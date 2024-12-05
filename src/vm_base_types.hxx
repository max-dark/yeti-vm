#pragma once

#include <stdlib.hxx>

namespace vm
{

using register_no = std::uint8_t;
constexpr register_no register_count = 32;
using register_t = std::uint32_t;
using signed_t = std::int32_t;
using unsigned_t = std::int32_t;
using register_file = std::array<register_t, register_count + 1>; // generic + PC

enum RegAlias: register_no
{
    zero = 0,
    ra = 1,
    sp = 2,
    gp = 3,
    tp = 4,
    t0 = 5,
    t1 = 6, t2 = 7,
    s0 = 8, fp = 8,
    s1 = 9,
    a0 = 10, a1 = 11,
    a2 = 12, a3 = 13, a4 = 14, a5 = 15, a6 = 16, a7 = 17,
    s2 = 18, s3 = 19, s4 = 20, s5 = 21, s6 = 22, s7 = 23, s8 = 24, s9 = 25, s10 = 26, s11 = 27,
    t3 = 28, t4 = 29, t5 = 30, t6 = 31,
    pc = register_count,
    unknown
};

} // namespace vm
