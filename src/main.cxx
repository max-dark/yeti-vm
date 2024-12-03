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
    data_t get_imm12() const
    {
        return get_bits(code, 20, 12);
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

constexpr bool have_ext_a(opcode_t code)
{
    switch (code) {
        case LUI: case AUIPC: case JAL: return false;
        default: return true;
    }
}

constexpr bool have_ext_b(opcode_t code)
{
    switch (code) {
        case LUI: case AUIPC: case JAL: return false;
        default: return true;
    }
}
} // namespace opcode

inline constexpr opcode::opcode_t no_func_a = 1 << 4;
inline constexpr opcode::opcode_t no_func_b = 1 << 8;
struct Code
{
    opcode::opcode_t code = 0;
    opcode::BaseFormat format = opcode::UNKNOWN;
    opcode::opcode_t funcA = no_func_a;
    opcode::opcode_t funcB = no_func_b;

    bool operator<(const Code& rhs) const
    {
        bool result = code < rhs.code;
        //result = result && (format < rhs.format);
        result = result && (funcA < rhs.funcA);
        result = result && (funcB < rhs.funcB);
        return result;
    }
};

struct interface
{
    using ptr = std::shared_ptr<interface>;
    virtual ~interface() = default;

    [[nodiscard]]
    virtual const Code& get_id() const = 0;

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

};

template
<
    opcode::opcode_t CodeBase,
    opcode::BaseFormat Format,
    opcode::opcode_t FuncA = no_func_a,
    opcode::opcode_t FuncB = no_func_b
>
struct instruction_base : public interface
{
    [[nodiscard]]
    const Code& get_id() const final
    {
        static const Code id{
            .code = CodeBase,
            .format = Format,
            .funcA = FuncA,
            .funcB = FuncB
        };

        return id;
    }

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
    using handler_ptr = const interface*;
    using type_map = std::map<Code, interface::ptr>;
    using base_map = std::map<opcode::opcode_t, type_map>;
    using format_arr = std::array<opcode::BaseFormat, 16>;
    using format_map = std::map<opcode::opcode_t, format_arr>;

    template<typename Handler>
    inline void register_handler()
    {
        static_assert(std::is_base_of_v<interface, Handler>, "Wrong type of Handler");
        register_handler(std::make_shared<Handler>());
    }
    void register_handler(interface::ptr handler)
    {
        auto& id = handler->get_id();
        auto& type = handlers[id.code];
        type[id] = std::move(handler);
    }

    handler_ptr find_handler(const opcode::OpcodeBase* code) const
    {
        auto op = code->get_code();
        const auto type = handlers.find(op);
        if (type != handlers.end())
        {
            auto& tmp = type->second;
        }

        return nullptr;
    }

    void register_format(opcode::BaseFormat fmt, opcode::opcode_t code, uint8_t func)
    {
        formats[code][func] = fmt;
    }

    [[nodiscard]]
    std::optional<opcode::BaseFormat> find_format(opcode::opcode_t code) const
    {
        opcode::OpcodeBase tmp{code};
        auto fmt = formats.find(code);
        if (fmt != formats.end()) return fmt->second[tmp.get_func3()];
        return std::nullopt;
    }

    base_map handlers;
    format_map formats;
};

namespace rv32i
{

struct lui: public instruction_base<opcode::LUI, opcode::U_TYPE> {};
struct auipc: public instruction_base<opcode::AUIPC, opcode::U_TYPE> {};

struct jal: public instruction_base<opcode::JAL, opcode::J_TYPE> {};
struct jalr: public instruction_base<opcode::JALR, opcode::I_TYPE> {};

template<opcode::opcode_t Type>
struct branch: public instruction_base<opcode::JALR, opcode::B_TYPE, Type> {};

struct beq : branch<0b0000> {};
struct bne : branch<0b0001> {};
struct blt : branch<0b0100> {};
struct bge : branch<0b0101> {};
struct bltu: branch<0b0110> {};
struct bgeu: branch<0b0111> {};

void add_branch(registry* r)
{
    r->register_handler<beq>();
    r->register_handler<bne>();
    r->register_handler<blt>();
    r->register_handler<bge>();
    r->register_handler<bltu>();
    r->register_handler<bgeu>();
}

template<opcode::opcode_t Type>
struct load: public instruction_base<opcode::LOAD, opcode::I_TYPE, Type> {};

struct lb : load<0b0000> {};
struct lh : load<0b0001> {};
struct lw : load<0b0010> {};
struct lbu: load<0b0100> {};
struct lhu: load<0b0101> {};

void add_load(registry* r)
{
    r->register_handler<lb>();
    r->register_handler<lh>();
    r->register_handler<lw>();
    r->register_handler<lbu>();
    r->register_handler<lhu>();
}

template<opcode::opcode_t Type>
struct store: public instruction_base<opcode::STORE, opcode::S_TYPE, Type> {};

struct sb: store<0b0000> {};
struct sh: store<0b0001> {};
struct sw: store<0b0010> {};

void add_store(registry* r)
{
    r->register_handler<sb>();
    r->register_handler<sh>();
    r->register_handler<sw>();
}

template<opcode::opcode_t Type>
struct int_imm: public instruction_base<opcode::OP_IMM, opcode::I_TYPE, Type> {};

struct sli : int_imm<0b0010> {};
struct sliu: int_imm<0b0011> {};
struct xori: int_imm<0b0100> {};
struct ori : int_imm<0b0110> {};
struct andi: int_imm<0b0111> {};

void add_int_imm(registry* r)
{
    r->register_handler<sli>();
    r->register_handler<sliu>();
    r->register_handler<xori>();
    r->register_handler<ori>();
    r->register_handler<andi>();
}

template<opcode::opcode_t Type, opcode::opcode_t Variant>
struct shift_imm: public instruction_base<opcode::OP_IMM, opcode::R_TYPE, Type, (Variant << 5)> {};

struct slli: shift_imm<0b0001, 0> {};
struct srli: shift_imm<0b0101, 0> {};
struct srai: shift_imm<0b0101, 1> {};

void add_shift_imm(registry* r)
{
    r->register_handler<slli>();
    r->register_handler<srli>();
    r->register_handler<srai>();
}

template<opcode::opcode_t Type, opcode::opcode_t Variant>
struct int_r: public instruction_base<opcode::OP, opcode::R_TYPE, Type, (Variant << 5)> {};

struct add_r : int_r<0b0000, 0> {};
struct sub_r : int_r<0b0000, 1> {};
struct sll_r : int_r<0b0001, 0> {};
struct slt_r : int_r<0b0010, 0> {};
struct sltu_r: int_r<0b0011, 0> {};
struct xor_r : int_r<0b0100, 0> {};
struct srl_r : int_r<0b0101, 0> {};
struct sra_r : int_r<0b0101, 1> {};
struct or_r  : int_r<0b0110, 0> {};
struct and_r : int_r<0b0111, 0> {};

void add_int(registry* r)
{
    r->register_handler<add_r>();
    r->register_handler<sub_r>();
    r->register_handler<sll_r>();
    r->register_handler<sll_r>();
    r->register_handler<slt_r>();
    r->register_handler<sltu_r>();
    r->register_handler<xor_r>();
    r->register_handler<srl_r>();
    r->register_handler<sra_r>();
    r->register_handler<or_r>();
    r->register_handler<and_r>();
}

template<opcode::opcode_t Type>
struct misc_mem: public instruction_base<opcode::MISC_MEM, opcode::I_TYPE, Type> {};

struct fence  : misc_mem<0b0000> {};
struct fence_i: misc_mem<0b0001> {};

void add_misc(registry* r)
{
    r->register_handler<fence>();
    r->register_handler<fence_i>();
}

// ECALL / EBREAK
struct env_call: public instruction_base<opcode::SYSTEM, opcode::I_TYPE> {};

template<opcode::opcode_t Type>
struct csr: public instruction_base<opcode::SYSTEM, opcode::I_TYPE, Type> {};

struct csrrw : csr<0b0001> {};
struct csrrs : csr<0b0010> {};
struct csrrc : csr<0b0011> {};
struct csrrwi: csr<0b0101> {};
struct csrrsi: csr<0b0110> {};
struct csrrci: csr<0b0111> {};

void add_system(registry* r)
{
    r->register_handler<env_call>();
    r->register_handler<csrrw>();
    r->register_handler<csrrs>();
    r->register_handler<csrrc>();
    r->register_handler<csrrwi>();
    r->register_handler<csrrsi>();
    r->register_handler<csrrci>();
}

void add_all(registry* r)
{
    r->register_handler<lui>();
    r->register_handler<auipc>();
    r->register_handler<jal>();
    r->register_handler<jalr>();

    add_branch(r);
    add_load(r);
    add_store(r);
    add_int_imm(r);
    add_shift_imm(r);
    add_int(r);
    add_misc(r);
    add_system(r);
}

} // namespace rv32i

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
