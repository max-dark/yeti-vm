#include "rv32i_handlers.hxx"
#include <yeti-vm/vm_handlers_rv32i.hxx>

#include <ranges>

namespace tests::rv32i
{
using namespace vm::rv32i;

using vm::bit_tools::bits;
using ::testing::StrictMock;
using MockType = ::testing::StrictMock<MockVM>;
using ::testing::_;
using ::testing::Args;
using ::testing::SetArgReferee;
using ::testing::Return;


/**
 * store data from RS2 to memory
 * immediate encodes sign-extended offset from RS1
 * funcA encodes size of data
 *
 * mem[rs1 + imm] = RS2
 */
class RV32I_Handler_Store
        : public RV32I_Handler
{
protected:
    using r_bits = bits<vm::register_t>;

    static Decoder encode(RegId base, RegId src, Offset offset, Code funcA)
    {
        return Decoder
        {
                Encoder::s_type(GroupId::STORE, base, src,
                            bits<Offset>::to_unsigned(offset), funcA)
        };
    }

    static vm::InstructionId expectedId(Code funcA)
    {
        return make_id(GroupId::STORE, Format::S_TYPE, funcA);
    }

    using StoreTest = void(MockVM &, vm::register_t, Address);
    static void testStore(vm::interface *impl, Code funcA, StoreTest storeTest)
    {
        EXPECT_TRUE(impl->get_id().equal(expectedId(funcA)));

        for (Offset offset: {-8, -4, 0, +4, +8})
        {
            for (RegId base = 0; base < vm::register_count; ++base)
            {
                for (RegId src = 0; src < vm::register_count; ++src)
                {
                    MockType mock;
                    vm::register_t base_addr = base * offset;
                    vm::register_t src_value = src * offset;
                    auto code = encode(base, src, offset, funcA);
                    EXPECT_CALL(mock, get_register(base))
                            .WillRepeatedly(Return(base_addr));
                    EXPECT_CALL(mock, get_register(src))
                            .WillRepeatedly(Return(src_value));
                    storeTest(mock, src_value, base_addr + offset);
                    impl->exec(&mock, &code);
                }
            }
        }
    }
};

TEST_F(RV32I_Handler_Store, StoreByte)
{
    ASSERT_TRUE(false) << "TODO: implementation";
}

TEST_F(RV32I_Handler_Store, StoreHalfWord)
{
    ASSERT_TRUE(false) << "TODO: implementation";
}

TEST_F(RV32I_Handler_Store, StoreWord)
{
    ASSERT_TRUE(false) << "TODO: implementation";
}

} //