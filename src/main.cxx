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
enum class Type: std::uint8_t
{
    UNKNOWN,
    R,
    I,
    S,
    B,
    U,
    J,
};
Type get_type(opcode_t code)
{
    return Type::UNKNOWN;
}

struct OpcodeBase
{
    opcode_t code;
};

template<Type type>
struct Opcode: public OpcodeBase
{
   static constexpr inline Type get_type() { return type; }
};

struct OpcodeR : public Opcode<Type::R>
{
    register_no rd;
    function_t f3;
    register_no rs1;
    register_no rs2;
    function_t f7;
};

struct OpcodeI : public Opcode<Type::I>
{
    register_no rd;
    function_t f3;
    register_no rs1;
    data_t imm; // [11:0]
};

struct OpcodeS : public Opcode<Type::S>
{};

struct OpcodeB : public Opcode<Type::B>
{};

struct OpcodeU : public Opcode<Type::U>
{};

struct OpcodeJ : public Opcode<Type::J>
{};

} // namespace opcode
} // namespace vm

int main(int argc, char** argv)
{
    return 0;
}