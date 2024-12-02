#include <iostream>
#include "stdlib.hxx"

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

std::string_view get_register_alias(register_no no)
{
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

    [[nodiscard]]
    opcode_t get_code() const
    {
        return base.op;
    }
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

enum BaseFormat: opcode_t
{
    R_TYPE,
    I_TYPE,
    S_TYPE,
    B_TYPE,
    U_TYPE,
    J_TYPE,
};

} // namespace opcode

namespace detail
{

struct interface
{
    using ptr = std::shared_ptr<interface>;
    virtual ~interface() = default;

    [[nodiscard]]
    virtual opcode::opcode_t get_code_base() const = 0;
    [[nodiscard]]
    virtual opcode::opcode_t get_func_a() const = 0;
    [[nodiscard]]
    virtual opcode::opcode_t get_func_b() const = 0;
    [[nodiscard]]
    virtual std::string_view get_mnemonic() const = 0;
    [[nodiscard]]
    virtual opcode::BaseFormat get_type() const = 0;

    template<uint8_t start, uint8_t length>
    static consteval opcode::data_t get_bits(opcode::opcode_t code)
    {
        constexpr uint8_t shift = (sizeof(code) * 8) - length;
        constexpr opcode::opcode_t mask = (~static_cast<opcode::opcode_t>(0)) >> shift;

        return (code >> start) & mask;
    }

    static constexpr opcode::data_t get_bits(opcode::opcode_t code, uint8_t start, uint8_t length)
    {
        uint8_t shift = (sizeof(code) * 8) - length;
        opcode::opcode_t mask = static_cast<opcode::opcode_t>(-1) >> shift;

        return (code >> start) & mask;
    }

    static register_no get_rs2(opcode::opcode_t code)
    {
        return get_bits(code, 20, 5);
    }

    static register_no get_rs1(opcode::opcode_t code)
    {
        return get_bits(code, 15, 5);
    }

    static register_no get_rd(opcode::opcode_t code)
    {
        return get_bits(code, 7, 5);
    }

    static opcode::data_t get_func3(opcode::opcode_t code)
    {
        return get_bits(code, 12, 3);
    }

    static opcode::data_t get_func7(opcode::opcode_t code)
    {
        return get_bits(code, 25, 7);
    }

    static opcode::data_t get_imm12(opcode::opcode_t code)
    {
        return get_bits(code, 20, 12);
    }
};
static_assert(interface::get_bits<0, 1>(0b0000'0001) == opcode::opcode_t{0b0000'0001}, "something wrong");
static_assert(interface::get_bits<0, 2>(0b0000'0011) == opcode::opcode_t{0b0000'0011}, "something wrong");
static_assert(interface::get_bits<1, 1>(0b0000'0010) == opcode::opcode_t{0b0000'0001}, "something wrong");

template
<
    opcode::opcode_t CodeBase,
    opcode::BaseFormat Format,
    opcode::opcode_t FuncA = 0,
    opcode::opcode_t FuncB = 0
>
struct instruction_base : public interface
{
    [[nodiscard]]
    opcode::opcode_t get_code_base() const final
    {
        return CodeBase;
    }

    [[nodiscard]]
    opcode::opcode_t get_func_a() const final
    {
        return FuncA;
    }

    [[nodiscard]]
    opcode::opcode_t get_func_b() const final
    {
        return FuncB;
    }

    [[nodiscard]]
    std::string_view get_mnemonic() const final
    {
        auto code = opcode::OpcodeType{get_code_base()};
        return opcode::get_op_id(code);
    }
    [[nodiscard]]
    opcode::BaseFormat get_type() const final
    {
        return Format;
    }
};

struct registry
{
    using handler_ptr = interface*;
    using type_map = std::map<opcode::BaseFormat, interface::ptr>;
    using base_map = std::map<opcode::opcode_t, type_map>;

    template<typename Handler>
    inline void register_handler()
    {
        static_assert(std::is_base_of_v<interface, Handler>, "Wrong type of Handler");
        register_handler(std::make_shared<Handler>());
    }
    void register_handler(interface::ptr handler)
    {
        auto& type = handlers[handler->get_code_base()];
        type[handler->get_type()] = std::move(handler);
    }

    handler_ptr find(opcode::opcode_t code)
    {
        return nullptr;
    }

    base_map handlers;
};

} // namespace detail
} // namespace vm

void disasm(const fs::path &program_file);
void disasm(const program &code);

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
        disasm(*bin);
    }
}

void disasm(const program &code)
{
    using op_type = vm::opcode::OpcodeBase;
    for (size_t i = 0; i < code.size(); i+= sizeof(op_type)) {
        const auto *p = code.data() + i;
        auto *op = reinterpret_cast<const op_type*>(p);
        std::cout << std::hex
                  << std::setw(8) << std::setfill('0') << std::right << op->code
                  << std::setw(10) << std::setfill(' ') << std::right << vm::opcode::get_op_id(static_cast<vm::opcode::OpcodeType>(op->get_code()))
                  << std::endl;
    }
}
