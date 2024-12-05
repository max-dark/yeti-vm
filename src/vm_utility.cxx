#include "vm_utility.hxx"

namespace vm
{

std::string_view get_register_alias(register_no no) {
    if (no > register_count) no = unknown;
    auto alias = static_cast<RegAlias>(no);

    switch (alias)
    {
        case zero:
            return "zero";
        case ra:
            return "ra";
        case sp:
            return "sp";
        case gp:
            return "gp";
        case tp:
            return "tp";
        case t0:
            return "t0/al";
        case t1:
            return "t1";
        case t2:
            return "t2";
        case s0:
            return "s0/fp";
        case s1:
            return "s1";
        case a0:
            return "a0/r0";
        case a1:
            return "a1/r1";
        case a2:
            return "a2";
        case a3:
            return "a3";
        case a4:
            return "a4";
        case a5:
            return "a5";
        case a6:
            return "a6";
        case a7:
            return "a7";
        case s2:
            return "s2";
        case s3:
            return "s3";
        case s4:
            return "s4";
        case s5:
            return "s5";
        case s6:
            return "s6";
        case s7:
            return "s7";
        case s8:
            return "s8";
        case s9:
            return "s9";
        case s10:
            return "s10";
        case s11:
            return "s11";
        case t3:
            return "t3";
        case t4:
            return "t4";
        case t5:
            return "t5";
        case t6:
            return "t6";
        case pc:
            return "pc";
        case unknown:
            return "unknown";
    }

    return "IMPOSSIBLE";
}
} // namespace vm
