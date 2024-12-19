#include "rv32i_handlers.hxx"
#include <yeti-vm/vm_handlers_rv32i.hxx>

namespace tests::rv32i
{
using namespace vm::rv32i;

using ::testing::_;
using ::testing::Return;

/**
 * @brief Integer Register-Register instructions(OP / R-type)
 */
class RV32I_Handler_RR
        : public RV32I_Handler
{
protected:
};

TEST_F(RV32I_Handler_RR, Add)
{
    FAIL() << "Not implemented";
}

TEST_F(RV32I_Handler_RR, Sub)
{
    FAIL() << "Not implemented";
}

TEST_F(RV32I_Handler_RR, Sll)
{
    FAIL() << "Not implemented";
}

TEST_F(RV32I_Handler_RR, Slt)
{
    FAIL() << "Not implemented";
}

TEST_F(RV32I_Handler_RR, SltU)
{
    FAIL() << "Not implemented";
}

TEST_F(RV32I_Handler_RR, Xor)
{
    FAIL() << "Not implemented";
}

TEST_F(RV32I_Handler_RR, Srl)
{
    FAIL() << "Not implemented";
}

TEST_F(RV32I_Handler_RR, Sra)
{
    FAIL() << "Not implemented";
}

TEST_F(RV32I_Handler_RR, Or)
{
    FAIL() << "Not implemented";
}

TEST_F(RV32I_Handler_RR, And)
{
    FAIL() << "Not implemented";
}

} // namespace tests::rv32i
