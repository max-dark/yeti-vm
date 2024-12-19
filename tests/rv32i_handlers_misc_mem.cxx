#include "rv32i_handlers.hxx"
#include <yeti-vm/vm_handlers_rv32i.hxx>

namespace tests::rv32i
{
using namespace vm::rv32i;

using ::testing::_;
using ::testing::Return;


class RV32I_Handler_MiscMem
        : public RV32I_Handler
{
protected:
    static Decoder encode(Code funcA)
    {
        Code code = Encoder::i_type(
                GroupId::MISC_MEM
                , 0, 0, 0
                , funcA
        );
        return Decoder{ code };
    }

    static vm::InstructionId expectedId(Code funcA)
    {
        return make_id(GroupId::MISC_MEM, Format::I_TYPE, funcA);
    }
};

TEST_F(RV32I_Handler_MiscMem, Fence)
{
    auto impl = create<fence>();
    constexpr Code funcA = 0b0000;

    ASSERT_TRUE(impl->get_id().equal(expectedId(funcA)));
    auto code = encode(funcA);
    MockVM mockVm;

    EXPECT_CALL(mockVm, barrier());
    impl->exec(&mockVm, &code);
}

TEST_F(RV32I_Handler_MiscMem, FenceI)
{
    auto impl = create<fence_i>();
    constexpr Code funcA = 0b0001;

    ASSERT_TRUE(impl->get_id().equal(expectedId(funcA)));
    auto code = encode(funcA);
    MockVM mockVm;

    EXPECT_CALL(mockVm, barrier());
    impl->exec(&mockVm, &code);
}


} // namespace tests::rv32i
