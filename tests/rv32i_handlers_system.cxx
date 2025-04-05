#include "rv32i_handlers.hxx"
#include <yeti-vm/vm_handlers_rv32i.hxx>

namespace tests::rv32i
{
using namespace vm::rv32i;

using ::testing::_;
using ::testing::Return;
using ::testing::SetArgReferee;
using ::testing::Sequence;
using ::testing::Expectation;


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

    ASSERT_TRUE(impl->getId().equal(expectedId(funcA)));
    auto code = encode(funcA, 0, 0, 0);
    MockVM mockVm;

    EXPECT_CALL(mockVm, syscall());
    impl->exec(&mockVm, &code);
}

TEST_F(RV32I_Handler_System, EnvBreak)
{
    auto impl = create<env_call>();
    constexpr Code funcA = 0b0000;

    ASSERT_TRUE(impl->getId().equal(expectedId(funcA)));
    auto code = encode(funcA, 0, 0, 1);
    MockVM mockVm;

    EXPECT_CALL(mockVm, debug());
    impl->exec(&mockVm, &code);
}

TEST_F(RV32I_Handler_System, CSR_RW)
{
    auto impl = create<csrrw>();
    constexpr Code funcA = 0b0001;

    ASSERT_TRUE(impl->getId().equal(expectedId(funcA)));
    for (register_no dst = 0; dst < vm::register_count; ++dst)
    {
        vm::register_t csr_id = 0;
        vm::register_t csr_val = 0;
        vm::register_t src_val = 0;
        register_no src = RegAlias::t0;
        auto code = encode(funcA, dst, src, csr_id);
        MockVM mockVm;

        Sequence csr;
        if (dst > 0) // should do not read CSR if dst == "zero"
        {
            EXPECT_CALL(mockVm, control_get(csr_id, _))
                    .InSequence(csr)
                    .WillOnce(SetArgReferee<1>(csr_val));
            EXPECT_CALL(mockVm, set_register(dst, csr_val))
                    .InSequence(csr);
        }
        EXPECT_CALL(mockVm, get_register(src))
                .InSequence(csr)
                .WillRepeatedly(Return(src_val));
        EXPECT_CALL(mockVm, control_set(csr_id, src_val))
                .InSequence(csr);
        impl->exec(&mockVm, &code);
    }
}

TEST_F(RV32I_Handler_System, CSR_RS)
{
    auto impl = create<csrrs>();
    constexpr Code funcA = 0b0010;

    ASSERT_TRUE(impl->getId().equal(expectedId(funcA)));
    for (register_no src = 0; src < vm::register_count; ++src) {
        vm::register_t csr_id = 0;
        vm::register_t csr_val = 0;
        vm::register_t src_val = 0;
        register_no dst = RegAlias::t0;
        auto code = encode(funcA, dst, src, csr_id);
        MockVM mockVm;

        Sequence csr_get, csr_set;
        Expectation was_read = EXPECT_CALL(mockVm, control_get(csr_id, _))
                .InSequence(csr_get);
        EXPECT_CALL(mockVm, set_register(dst, _))
                .InSequence(csr_get);
        if (src == 0)
        {
            EXPECT_CALL(mockVm, control_set(_, _))
                .Times(0);
        }
        else
        {
            EXPECT_CALL(mockVm, get_register(src))
                    .InSequence(csr_set)
                    .WillOnce(Return(0));
            EXPECT_CALL(mockVm, control_set(csr_id, _))
                    .InSequence(csr_set)
                    .After(was_read);
        }
        impl->exec(&mockVm, &code);
    }
}

TEST_F(RV32I_Handler_System, CSR_RC)
{
    auto impl = create<csrrc>();
    constexpr Code funcA = 0b0011;

    ASSERT_TRUE(impl->getId().equal(expectedId(funcA)));
    for (register_no src = 0; src < vm::register_count; ++src) {
        vm::register_t csr_id = 0;
        vm::register_t csr_val = 0;
        vm::register_t src_val = 0;
        register_no dst = RegAlias::t0;
        auto code = encode(funcA, dst, src, csr_id);
        MockVM mockVm;

        Sequence csr_get, csr_set;
        Expectation was_read = EXPECT_CALL(mockVm, control_get(csr_id, _))
                .InSequence(csr_get);
        EXPECT_CALL(mockVm, set_register(dst, _))
                .InSequence(csr_get);
        if (src == 0)
        {
            EXPECT_CALL(mockVm, control_set(_, _))
                    .Times(0);
        }
        else
        {
            EXPECT_CALL(mockVm, get_register(src))
                    .InSequence(csr_set)
                    .WillOnce(Return(0));
            EXPECT_CALL(mockVm, control_set(csr_id, _))
                    .InSequence(csr_set)
                    .After(was_read);
        }
        impl->exec(&mockVm, &code);
    }
}

TEST_F(RV32I_Handler_System, CSR_RW_I)
{
    auto impl = create<csrrwi>();
    constexpr Code funcA = 0b0101;

    ASSERT_TRUE(impl->getId().equal(expectedId(funcA)));
    for (register_no dst = 0; dst < vm::register_count; ++dst)
    {
        vm::register_t csr_id = 0;
        register_no src = 0xef & 0b0001'1111; // uImm[4:0]
        auto code = encode(funcA, dst, src, csr_id);
        MockVM mockVm;

        Sequence csr;
        if (dst > 0) // should do not read CSR if dst == "zero"
        {
            EXPECT_CALL(mockVm, control_get(csr_id, _))
                    .InSequence(csr);
            EXPECT_CALL(mockVm, set_register(dst, _))
                    .InSequence(csr);
        }
        EXPECT_CALL(mockVm, control_set(csr_id, src))
                .InSequence(csr);
        impl->exec(&mockVm, &code);
    }
}

TEST_F(RV32I_Handler_System, CSR_RS_I)
{
    auto impl = create<csrrsi>();
    constexpr Code funcA = 0b0110;

    ASSERT_TRUE(impl->getId().equal(expectedId(funcA)));
    for (register_no src = 0; src < vm::register_count; ++src) {
        vm::register_t csr_id = 0;
        vm::register_t csr_val = 0;
        vm::register_t src_val = 0;
        register_no dst = RegAlias::t0;
        auto code = encode(funcA, dst, src, csr_id);
        MockVM mockVm;

        Sequence csr_get;
        Expectation was_read = EXPECT_CALL(mockVm, control_get(csr_id, _))
                .InSequence(csr_get);
        EXPECT_CALL(mockVm, set_register(dst, _))
                .InSequence(csr_get);
        if (src == 0)
        {
            EXPECT_CALL(mockVm, control_set(_, _))
                    .Times(0);
        }
        else
        {
            EXPECT_CALL(mockVm, control_set(csr_id, _))
                    .After(was_read);
        }
        impl->exec(&mockVm, &code);
    }
}

TEST_F(RV32I_Handler_System, CSR_RC_I)
{
    auto impl = create<csrrci>();
    constexpr Code funcA = 0b0111;

    ASSERT_TRUE(impl->getId().equal(expectedId(funcA)));
    for (register_no src = 0; src < vm::register_count; ++src) {
        vm::register_t csr_id = 0;
        vm::register_t csr_val = 0;
        vm::register_t src_val = 0;
        register_no dst = RegAlias::t0;
        auto code = encode(funcA, dst, src, csr_id);
        MockVM mockVm;

        Sequence csr_get;
        Expectation was_read = EXPECT_CALL(mockVm, control_get(csr_id, _))
                .InSequence(csr_get);
        EXPECT_CALL(mockVm, set_register(dst, _))
                .InSequence(csr_get);
        if (src == 0)
        {
            EXPECT_CALL(mockVm, control_set(_, _))
                    .Times(0);
        }
        else
        {
            EXPECT_CALL(mockVm, control_set(csr_id, _))
                    .After(was_read);
        }
        impl->exec(&mockVm, &code);
    }
}


} // namespace tests::rv32i
