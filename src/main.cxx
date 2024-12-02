#include <iostream>
#include "stdlib.hxx"


void disasm(const fs::path &program_file);

namespace fs = std::filesystem;

using program = std::vector<std::uint8_t>;

std::optional<program> load_program(const fs::path& programFile)
{
    program data;

    if (fs::is_regular_file(programFile))
    {
        auto sz = static_cast<std::streamsize>(fs::file_size(programFile));
        data.resize(sz);
        std::ifstream bin{programFile, std::ios::binary};

        if (bin.is_open())
        {
            bin.read(reinterpret_cast<char *>(data.data()), sz);
            if (bin.good() || bin.eof())
                return data;
        }
    }

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

data_t extend_sign(data_t value, opcode_t code)
{
    data_t sign = code & (1u << 31);
    while (sign != (value & sign))
    {
        value |= sign;
        sign >>= 1;
    }
    return value;
}

struct OpcodeBase
{
    using bf = data_t;

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
    LOAD   = make_opcode(0b00, 0b000), // read value from memory
    STORE  = make_opcode(0b01, 0b000), // write value to memory
    MADD   = make_opcode(0b10, 0b000), // "Multiple and Add"(float/double)
    BRANCH = make_opcode(0b11, 0b000), // conditional jump

    LOAD_FP  = make_opcode(0b00, 0b001), // read value from memory(float/double)
    STORE_FP = make_opcode(0b01, 0b001), // write value to memory(float/double)
    MSUB     = make_opcode(0b10, 0b001), // "Multiple and Sub"(float/double)
    JALR     = make_opcode(0b11, 0b001), // jump to offset from register

    CUSTOM_0 = make_opcode(0b00, 0b010), // for extensions
    CUSTOM_1 = make_opcode(0b01, 0b010), // for extensions
    NMSUB    = make_opcode(0b10, 0b010), // "Multiple and Sub"(float/double)
    // R_11_010 = make_opcode(0b11, 0b010), // reserved

    MISC_MEM = make_opcode(0b00, 0b011), // sync, barriers, etc
    AMO      = make_opcode(0b01, 0b011), // atomic ops
    NMADD    = make_opcode(0b10, 0b011), // "Multiple and Add"(float/double)
    JAL      = make_opcode(0b11, 0b011), // jump to offset

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

std::string_view get_op_id(OpcodeType code)
{
    switch (code)
    {
        case LOAD:
            return "LOAD";
        case STORE:
            return "STORE";
        case MADD:
            return "MADD";
        case BRANCH:
            return "BRANCH";
        case LOAD_FP:
            return "LOAD_FP";
        case STORE_FP:
            return "STORE_FP";
        case MSUB:
            return "MSUB";
        case JALR:
            return "JALR";
        case CUSTOM_0:
            return "CUSTOM_0";
        case CUSTOM_1:
            return "CUSTOM_1";
        case MISC_MEM:
            return "MISC_MEM";
        case AMO:
            return "AMO";
        case JAL:
            return "JAL";
        case OP_IMM:
            return "OP_IMM";
        case OP:
            return "OP";
        case SYSTEM:
            return "SYSTEM";
        case AUIPC:
            return "AUIPC";
        case LUI:
            return "LUI";
        case OP_IMM_32:
            return "OP_IMM_32";
        case OP_32:
            return "OP_32";
        case CUSTOM_3:
            return "CUSTOM_3";
        case CUSTOM_2:
            return "CUSTOM_2";
        case NMSUB:
            return "NMSUB";
        case NMADD:
            return "NMADD";
        case OP_FP:
            return "OP_FP";
    }
    return "UNKNOWN";
}

} // namespace opcode
} // namespace vm

void disasm(const fs::path &program_file);

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cout << "Usage: exe <path/to/file.bin>" << std::endl;
        return 0;
    }

    fs::path program_file = argv[1];
    disasm(program_file);

    return 0;
}

void disasm(const fs::path &program_file)
{
    auto bin = load_program(program_file);

    if (bin)
    {
        using op_type = vm::opcode::OpcodeBase;
        for (size_t i = 0; i < bin->size(); i+= sizeof(op_type)) {
            const auto *p = bin->data() + i;
            auto *op = reinterpret_cast<const op_type*>(p);
            std::cout << std::hex
                << std::setw(8) << std::setfill('0') << std::right << op->code
                << std::setw(10) << std::setfill(' ') << std::right << vm::opcode::get_op_id(static_cast<vm::opcode::OpcodeType>(op->base.op))
                << std::endl;
        }
    }
}
