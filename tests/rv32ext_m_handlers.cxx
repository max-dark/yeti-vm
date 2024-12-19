/// RV32 'M' extension tests

#include "rv32_vm_mocks.hxx"

#include <yeti-vm/vm_handlers_rv32m.hxx>

#include <ranges>

namespace tests::rv32m
{
using namespace vm::rv32m;
using vm::bit_tools::bits;

using ::testing::_;
using ::testing::Expectation;
using ::testing::Return;
using ::testing::Sequence;

using namespace tests::rv32_vm;

using RegId = vm::register_no;
using Value = vm::register_t;
using GroupId = vm::opcode::OpcodeType;
using Format = vm::opcode::BaseFormat;
using vm::opcode::Decoder;
using vm::opcode::Encoder;
using Code = vm::opcode::opcode_t;
using ExtId = Code;

template<class Handler>
concept Implementation = std::is_base_of_v<vm::interface, Handler>;
using HandlerPtr = std::unique_ptr<vm::interface>;

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

    template<Implementation Type>
    [[nodiscard]]
    static auto create()
    {
        return std::make_unique<Type>();
    }

    static void integrationTest(vm::interface* impl, ExtId extId)
    {
        ASSERT_TRUE(impl->get_id().equal(expectedId(extId)));

        MockVM mockVm;

        RegId id_dst = 1;
        RegId id_lhs = 2;
        RegId id_rhs = 3;

        Value res = 0;
        Value lhs = 0;
        Value rhs = 0;

        Decoder code = encode(id_dst, id_lhs, id_rhs, extId);

        Expectation get_lhs = EXPECT_CALL(mockVm, get_register(id_lhs))
            .WillOnce(Return(lhs));
        Expectation get_rhs = EXPECT_CALL(mockVm, get_register(id_rhs))
            .WillOnce(Return(rhs));

        EXPECT_CALL(mockVm, set_register(id_dst, _))
            .Times(1)
            .After(get_lhs, get_rhs);

        impl->exec(&mockVm, &code);
    }

    void SetUp() override
    {
        //disableUnitTestWarning = true;
    }
protected:
    bool disableUnitTestWarning = false;
};

TEST_F(RV32Ext_IntMath, IntMul)
{
    auto impl = create<mul>();
    EXPECT_TRUE(disableUnitTestWarning) << impl->get_mnemonic() << " Unit test is not implemented";
    return integrationTest(impl.get(), 0b0000);
}

TEST_F(RV32Ext_IntMath, IntMulH)
{
    auto impl = create<mulh>();
    EXPECT_TRUE(disableUnitTestWarning) << impl->get_mnemonic() << " Unit test is not implemented";
    return integrationTest(impl.get(), 0b0001);
}

TEST_F(RV32Ext_IntMath, IntMulHSU)
{
    auto impl = create<mulhsu>();
    EXPECT_TRUE(disableUnitTestWarning) << impl->get_mnemonic() << " Unit test is not implemented";
    return integrationTest(impl.get(), 0b0010);
}

TEST_F(RV32Ext_IntMath, IntMulHU)
{
    auto impl = create<mulhu>();
    EXPECT_TRUE(disableUnitTestWarning) << impl->get_mnemonic() << " Unit test is not implemented";
    return integrationTest(impl.get(), 0b0011);
}

TEST_F(RV32Ext_IntMath, IntDiv)
{
    auto impl = create<vm::rv32m::div>();
    EXPECT_TRUE(disableUnitTestWarning) << impl->get_mnemonic() << " Unit test is not implemented";
    return integrationTest(impl.get(), 0b0100);
}

TEST_F(RV32Ext_IntMath, IntDivU)
{
    auto impl = create<divu>();
    EXPECT_TRUE(disableUnitTestWarning) << impl->get_mnemonic() << " Unit test is not implemented";
    return integrationTest(impl.get(), 0b0101);
}

TEST_F(RV32Ext_IntMath, IntRem)
{
    auto impl = create<rem>();
    EXPECT_TRUE(disableUnitTestWarning) << impl->get_mnemonic() << " Unit test is not implemented";
    return integrationTest(impl.get(), 0b0110);
}

TEST_F(RV32Ext_IntMath, IntRemU)
{
    auto impl = create<remu>();
    EXPECT_TRUE(disableUnitTestWarning) << impl->get_mnemonic() << " Unit test is not implemented";
    return integrationTest(impl.get(), 0b0111);
}

} // namespace tests::rv32m
