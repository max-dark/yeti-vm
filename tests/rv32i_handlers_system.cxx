#include "rv32i_handlers.hxx"
#include <yeti-vm/vm_handlers_rv32i.hxx>

namespace tests::rv32i
{
using namespace vm::rv32i;

using ::testing::_;
using ::testing::Return;


class RV32I_Handler_System
        : public RV32I_Handler
{
protected:
    static Decoder encode(Code funcA, RegId rd, RegId rs1, Code value)
    {
        Code code = Encoder::i_type(
                GroupId::SYSTEM
                , rd, rs1, value
                , funcA
        );
        return Decoder{ code };
    }

    static vm::InstructionId expectedId(Code funcA)
    {
        return make_id(GroupId::SYSTEM, Format::I_TYPE, funcA);
    }
};

TEST_F(RV32I_Handler_System, EnvCall)
{
    auto impl = create<env_call>();
    constexpr Code funcA = 0b0000;

    ASSERT_TRUE(impl->get_id().equal(expectedId(funcA)));
    auto code = encode(funcA, 0, 0, 0);
    MockVM mockVm;

    EXPECT_CALL(mockVm, syscall());
    impl->exec(&mockVm, &code);
}

TEST_F(RV32I_Handler_System, EnvBreak)
{
    auto impl = create<env_call>();
    constexpr Code funcA = 0b0000;

    ASSERT_TRUE(impl->get_id().equal(expectedId(funcA)));
    auto code = encode(funcA, 0, 0, 1);
    MockVM mockVm;

    EXPECT_CALL(mockVm, debug());
    impl->exec(&mockVm, &code);
}


} // namespace tests::rv32i
