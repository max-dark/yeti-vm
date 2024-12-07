#include "vm_opcode.hxx"

namespace vm::opcode
{

data_t extend_sign(data_t value, opcode_t code) {
    data_t sign = code & (1u << 31);
    while (sign != (value & sign))
    {
        value |= sign;
        sign >>= 1;
    }
    return value;
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
    }
    return "UNKNOWN";
}

data_t OpcodeBase::decode_i() const {
    return shift_bits<20, 0, 12>(code);
}

data_t OpcodeBase::decode_s() const {
    auto a = shift_bits< 7, 0, 5>(code);
    auto b = shift_bits<25, 5, 7>(code);
    return a | b;
}

data_t OpcodeBase::decode_b() const {
    auto s = shift_bits<31, 12, 1>(code);
    auto a = shift_bits< 8,  1, 4>(code);
    auto c = shift_bits< 7, 11, 1>(code);
    auto b = shift_bits<25,  5, 6>(code);
    return s | a | b | c | 0;
}

data_t OpcodeBase::decode_u() const {
    return code & mask_value<12, 30>;
}

data_t OpcodeBase::decode_j() const {
    auto s = shift_bits<31, 20, 1>(code); // [31]
    auto a = shift_bits<12, 12, 8>(code); // [19:12]
    auto b = shift_bits<20, 11, 1>(code); // [20]
    auto c = shift_bits<21,  1,10>(code); // [30:21]
    return s | a | b | c | 0;
}
} //namespace vm::opcode
