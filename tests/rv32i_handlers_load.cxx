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
using ::testing::Sequence;
using ::testing::Return;

/**
 * load data from memory to RD
 * immediate encodes sign-extended offset from RS1
 * funcA encodes type of data
 *
 * rd = mem[rs1 + imm]
 */
class RV32I_Handler_Load
        : public RV32I_Handler
{
protected:
    static Decoder encode(RegId dest, RegId base, Offset offset, Code funcA)
    {
        return Decoder
        {
            Encoder::i_type(GroupId::LOAD
                                , dest, base,
                            bits<Offset>::to_unsigned(offset)
                                , funcA)
        };
    }

    static vm::InstructionId expectedId(Code funcA)
    {
        return make_id(GroupId::LOAD, Format::I_TYPE, funcA);
    }

    using LoadTest = vm::register_t(MockVM&, Sequence&, vm::register_t, Address);
    static void testLoad(vm::interface* impl, Code funcA, LoadTest loadTest)
    {
        ASSERT_TRUE(impl->get_id().equal(expectedId(funcA)));

        for (Offset offset: { -8, -4, 0, +4, +8 })
        {
            for (RegId dest = 0; dest < vm::register_count; ++dest)
            {
                for (RegId base = 0; base < vm::register_count; ++base)
                {
                    Sequence get_block;
                    MockType mock;
                    vm::register_t rs1_value = dest * offset;
                    vm::register_t rs2_value = base * offset;
                    auto code = encode(dest, base, offset, funcA);
                    EXPECT_CALL(mock, get_register(base))
                            .InSequence(get_block)
                            .WillRepeatedly(Return(rs2_value));

                    vm::register_t value = loadTest(mock, get_block, rs1_value, rs2_value + offset);
                    EXPECT_CALL(mock, set_register(dest, value))
                        .InSequence(get_block);
                    impl->exec(&mock, &code);
                }
            }
        }
    }

    using r_bits = bits<vm::register_t>;
};

TEST_F(RV32I_Handler_Load, ByteSigned)
{
    Code funcA = 0b0000;
    testLoad(create<lb>(), funcA
             ,[](MockVM& vm, Sequence& s
             , vm::register_t value, Address address)
    {
        EXPECT_CALL(vm, read_memory(address, 1, _))
            .InSequence(s)
            .WillRepeatedly(SetArgReferee<2>(value & 0x00ff))
        ;
        return r_bits::extend_sign<7>(value & 0x00ff);
    });
}

TEST_F(RV32I_Handler_Load, HalfWordSigned)
{
    Code funcA = 0b0001;
    testLoad(create<lh>(), funcA
            ,[](MockVM& vm, Sequence& s
            , vm::register_t value, Address address)
    {
     EXPECT_CALL(vm, read_memory(address, 2, _))
             .InSequence(s)
             .WillRepeatedly(SetArgReferee<2>(value & 0x00ffff))
             ;
     return r_bits::extend_sign<15>(value & 0x00ffff);
    });
}

TEST_F(RV32I_Handler_Load, Word)
{
    Code funcA = 0b0010;
    testLoad(create<lw>(), funcA
            ,[](MockVM& vm, Sequence& s
            , vm::register_t value, Address address)
    {
     EXPECT_CALL(vm, read_memory(address, 4, _))
             .InSequence(s)
             .WillRepeatedly(SetArgReferee<2>(value))
             ;
     return value;
    });
}

TEST_F(RV32I_Handler_Load, ByteUnsigned)
{
    Code funcA = 0b0100;
    testLoad(create<lbu>(), funcA
            ,[](MockVM& vm, Sequence& s
            , vm::register_t value, Address address)
    {
         EXPECT_CALL(vm, read_memory(address, 1, _))
                 .InSequence(s)
                 .WillRepeatedly(SetArgReferee<2>(value & 0x00ff))
                 ;
         return (value & 0x00ff);
    });
}

TEST_F(RV32I_Handler_Load, HalfWordUnsigned)
{
    Code funcA = 0b0101;
    testLoad(create<lhu>(), funcA
            ,[](MockVM& vm, Sequence& s
            , vm::register_t value, Address address)
    {
     EXPECT_CALL(vm, read_memory(address, 2, _))
             .InSequence(s)
             .WillRepeatedly(SetArgReferee<2>(value & 0x00ffff))
             ;
     return (value & 0x00ffff);
    });
}

} // namespace tests::rv32i
