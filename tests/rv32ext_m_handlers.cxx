/// RV32 'M' extension tests

#include "rv32_vm_mocks.hxx"

#include <yeti-vm/vm_handlers_rv32m.hxx>

#include <ranges>

namespace tests::rv32m
{
using namespace vm::rv32m;
using vm::bit_tools::bits;

using ::testing::_;
using ::testing::Return;

using namespace tests::rv32_vm;

using RegId = vm::register_no;
using Value = vm::register_t;
using GroupId = vm::opcode::OpcodeType;
using Format = vm::opcode::BaseFormat;
using vm::opcode::Decoder;
using vm::opcode::Encoder;
using Code = vm::opcode::opcode_t;
using ExtId = Code;

/**
 * "M" Extension for Integer Multiplication and Division
 */
class RV32Ext_IntMath: public ::testing::Test
{
protected:
    static constexpr ExtId funcB = 0b000'0001;
    static Decoder encode(RegId dest, RegId lhs, RegId rhs, ExtId funcA)
    {
        Code code = Encoder::r_type(
                GroupId::OP
                , dest, lhs, rhs
                , funcA, funcB
        );
        return Decoder{ code };
    }

    static vm::InstructionId expectedId(ExtId funcA)
    {
        return {
                GroupId::OP, Format::R_TYPE,
                funcA, funcB
        };
    }
};

TEST_F(RV32Ext_IntMath, IntMul)
{
    FAIL() << "Not implemented";
}

TEST_F(RV32Ext_IntMath, IntMulH)
{
    FAIL() << "Not implemented";
}

TEST_F(RV32Ext_IntMath, IntMulHSU)
{
    FAIL() << "Not implemented";
}

TEST_F(RV32Ext_IntMath, IntMulHU)
{
    FAIL() << "Not implemented";
}

TEST_F(RV32Ext_IntMath, IntDiv)
{
    FAIL() << "Not implemented";
}

TEST_F(RV32Ext_IntMath, IntDivU)
{
    FAIL() << "Not implemented";
}

TEST_F(RV32Ext_IntMath, IntRem)
{
    FAIL() << "Not implemented";
}

TEST_F(RV32Ext_IntMath, IntRemU)
{
    FAIL() << "Not implemented";
}

} // namespace tests::rv32m
