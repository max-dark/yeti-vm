#include "vm_opcode.hxx"

namespace vm::opcode
{

data_t extend_sign(data_t value, opcode_t code) {
    data_t sign = code & (1u << 31);
    return sign ? bit_tools::bits<data_t>::extend_sign(value) : value;
}

size_t op_size(opcode_t code) {
    if ((0b00000011 & code) != 0b0000011) return 1;
    if ((0b00011100 & code) != 0b0011100) return 2;
    if ((0b00111111 & code) == 0b0011111) return 3;
    if ((0b01111111 & code) == 0b0111111) return 3;

    return 0;
}

std::string_view get_op_id(OpcodeType code) {
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
        case R_11_010:
            return "R_11_010";
        case R_10_101:
            return "R_10_101";
        case R_11_101:
            return "R_11_101";
    }
    return "UNKNOWN";
}

data_t Decoder::decode_i() const {
    return extend_sign<11>(decode_i_u());
}

data_t Decoder::decode_i_u() const {
    return shift_bits<20, 0, 12>(code);
}

data_t Decoder::decode_s() const {
    return extend_sign<11>(decode_s_u());
}

data_t Decoder::decode_s_u() const
{
    auto a = shift_bits< 7, 0, 5>(code);
    auto b = shift_bits<25, 5, 7>(code);
    return (a | b);
}

data_t Decoder::decode_b() const {
    return extend_sign<12>(decode_b_u());
}

data_t Decoder::decode_b_u() const {
    auto s = shift_bits<31, 12, 1>(code);
    auto a = shift_bits< 8,  1, 4>(code);
    auto c = shift_bits< 7, 11, 1>(code);
    auto b = shift_bits<25,  5, 6>(code);
    return (s | a | b | c | 0);
}

data_t Decoder::decode_u() const {
    return code & make_mask<12, 30>();
}

data_t Decoder::decode_j() const {
    return extend_sign<20>(decode_j_u());
}

data_t Decoder::decode_j_u() const {
    auto s = shift_bits<31, 20, 1>(code); // [31]
    auto a = shift_bits<12, 12, 8>(code); // [19:12]
    auto b = shift_bits<20, 11, 1>(code); // [20]
    auto c = shift_bits<21,  1,10>(code); // [30:21]
    return (s | a | b | c | 0);
}

Encoder::instruction_t Encoder::r_type(Encoder::base_t group,
                                       Encoder::reg_id rd,
                                       Encoder::reg_id rs1,
                                       Encoder::reg_id rs2,
                                       Encoder::func_id fa,
                                       Encoder::func_id fb)
{
    instruction_t value = 0;
    value |= encode_group(group);
    value |= encode_rd(rd);
    value |= encode_f3(fa);
    value |= encode_rs1(rs1);
    value |= encode_rs2(rs2);
    value |= encode_f7(fb);

    return value;
}

Encoder::instruction_t Encoder::i_type(Encoder::base_t group,
                                       Encoder::reg_id rd,
                                       Encoder::reg_id rs1,
                                       Encoder::immediate_t immediate,
                                       Encoder::func_id fa)
{
    instruction_t value = 0;
    value |= encode_group(group);
    value |= encode_rd(rd);
    value |= encode_f3(fa);
    value |= encode_rs1(rs1);

    return value | encode_i(immediate);
}

Encoder::instruction_t Encoder::s_type(Encoder::base_t group,
                                       Encoder::reg_id rs1,
                                       Encoder::reg_id rs2,
                                       Encoder::immediate_t immediate,
                                       Encoder::func_id fa)
{
    instruction_t value = 0;
    value |= encode_group(group);
    value |= encode_f3(fa);
    value |= encode_rs1(rs1);
    value |= encode_rs2(rs2);

    return value | encode_s(immediate);
}

Encoder::instruction_t Encoder::b_type(Encoder::base_t group,
                                       Encoder::reg_id rs1,
                                       Encoder::reg_id rs2,
                                       Encoder::immediate_t immediate,
                                       Encoder::func_id fa)
{
    instruction_t value = 0;
    value |= encode_group(group);
    value |= encode_f3(fa);
    value |= encode_rs1(rs1);
    value |= encode_rs2(rs2);

    return value | encode_b(immediate);
}

Encoder::instruction_t
Encoder::u_type(Encoder::base_t group, Encoder::reg_id rd, Encoder::immediate_t immediate)
{
    instruction_t value = 0;
    value |= encode_group(group);
    value |= encode_rd(rd);

    return value | encode_u(immediate);
}

Encoder::instruction_t
Encoder::j_type(Encoder::base_t group, Encoder::reg_id rd, Encoder::immediate_t immediate)
{
    instruction_t value = 0;
    value |= encode_group(group);
    value |= encode_rd(rd);

    return value | encode_j(immediate);
}

Encoder::instruction_t Encoder::encode_i(immediate_t immediate)
{
    return immediate << 20;
}

Encoder::instruction_t Encoder::encode_s(Encoder::immediate_t immediate)
{
    instruction_t value = 0;
    value |= get_range<0,  4>(immediate) << 7;
    value |= get_range<5, 11>(immediate) << 25;

    return value;
}

Encoder::instruction_t Encoder::encode_b(Encoder::immediate_t immediate)
{
    instruction_t value = 0;
    value |= get_range<11>(immediate) << 7;
    value |= get_range<1, 4>(immediate) << 8;
    value |= get_range<5, 10>(immediate) << 25;
    value |= get_range<12>(immediate) << 31;

    return value;
}

Encoder::instruction_t Encoder::encode_u(Encoder::immediate_t immediate)
{
    instruction_t value = 0;
    value |= immediate << 12;

    return value;
}

Encoder::instruction_t Encoder::encode_j(Encoder::immediate_t immediate)
{
    instruction_t value = 0;
    value |= get_range<20>(immediate) << 31;
    value |= get_range<1, 10>(immediate) << 21;
    value |= get_range<11>(immediate) << 20;
    value |= get_range<12, 19>(immediate) << 12;

    return value;
}

} //namespace vm::opcode
