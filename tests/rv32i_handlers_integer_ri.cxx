#include "rv32i_handlers.hxx"
#include <yeti-vm/vm_handlers_rv32i.hxx>

#include <ranges>

namespace tests::rv32i
{
using namespace vm::rv32i;

using ::testing::_;
using ::testing::Return;

/**
 * @brief Integer Register-Immediate instructions(OP_IMM / I-type)
 *
 * Immediate encodes I-type sign-extended `const`.
 * @code rd = rs1 OP const @endcode
 *
 * Shift operations uses specialisation of I-type format
 * const[4:0] - shift amount
 * const[11:5] - subtype:
 *      const[30] == 0 - logical
 *      const[30] == 1 - arithmetic
 *
 * @sa vm::rv32i::lui
 * @sa vm::rv32i::auipc
 * @sa RV32I_Handler_Impl
 */
class RV32I_Handler_RI
        : public RV32I_Handler
{
protected:
    static void NotImplemented()
    {
        FAIL() << "not implemented";
    }

    static Decoder encode(RegId dest, RegId src, Code value, Code funcA)
    {
        value &= r_bits::mask_value<0, 12>;
        Code code = Encoder::i_type(GroupId::OP_IMM, dest, src, value, funcA);
        return Decoder(code);
    }

    static vm::InstructionId expectedId(Code funcA, Format format = Format::I_TYPE, Code funcB = NoFuncB)
    {
        return make_id(GroupId::OP_IMM, format, funcA, funcB);
    }
    static void AssertId(const vm::interface* impl, const vm::InstructionId& expected)
    {
        ASSERT_TRUE(impl->get_id().equal(expected));
    }

    struct TestParams
    {
        RegId dest;
        RegId src;
        Code data;
    };

    struct TestValues
    {
        Decoder code;
        vm::register_t dest;
        vm::register_t src;
    };

    using TestStep = std::function<TestValues(const vm::interface* impl, const TestParams* thisTest)>;
    static void commonTest(const vm::interface* impl, Code funcA, const TestStep& step)
    {
        AssertId(impl, expectedId(funcA));

        for (RegId dest = 0; dest < vm::register_count; ++dest)
        {
            for (RegId src = 0; src < vm::register_count; ++src)
            for (Offset offset: { -8, -4, 0, +4, +8 })
            {
                MockVM mockVm;
                TestParams thisTest{};
                thisTest.dest = dest;
                thisTest.src = src;
                thisTest.data = offset;

                const TestValues r = step(impl, &thisTest);
                ShouldGetRegister(mockVm, src, r.src);
                ShouldSetRegister(mockVm, dest, r.dest);

                impl->exec(&mockVm, &r.code);
            }
        }
    }

    static void ShouldGetRegister(MockVM& mockVm, RegId id, vm::register_t value)
    {
        EXPECT_CALL(mockVm, get_register(id))
            .WillRepeatedly(Return(value))
            ;
    }
    static void ShouldSetRegister(MockVM& mockVm, RegId id, vm::register_t value)
    {
        EXPECT_CALL(mockVm, set_register(id, value));
    }

    static void DecoderShouldReturnSameValue(const TestValues& r, const TestParams* p, Code funcA)
    {
        ASSERT_EQ(r.code.decode_i(), p->data);
        ASSERT_EQ(r.code.get_rd(), p->dest);
        ASSERT_EQ(r.code.get_rs1(), p->src);
        ASSERT_EQ(r.code.get_func3(), funcA);
    };
};

TEST_F(RV32I_Handler_RI, AddImmediate)
{
    static constexpr Code funcA = 0b0000;
    auto impl = create<addi>();
    TestStep step = [](const vm::interface* impl, const TestParams* p) -> TestValues
    {
        TestValues r{};
        r.code = encode(p->dest, p->src, p->data, funcA);
        DecoderShouldReturnSameValue(r, p, funcA);
        r.src = p->src * (p->dest ^ p->data);
        r.dest = r.src + r_bits::to_signed(r.code.decode_i());

        return r;
    };
    return commonTest(impl, funcA, step);
}

TEST_F(RV32I_Handler_RI, SetLessThanImmediate)
{
    static constexpr Code funcA = 0b0010;
    auto impl = create<slti>();
    TestStep step = [](const vm::interface* impl, const TestParams* p) -> TestValues
    {
        TestValues r{};
        r.code = encode(p->dest, p->src, p->data, funcA);
        DecoderShouldReturnSameValue(r, p, funcA);
        r.src = p->src * (p->dest ^ p->data);
        r.dest = r_bits::to_signed(r.src) < r_bits::to_signed(r.code.decode_i());

        return r;
    };
    return commonTest(impl, funcA, step);
}

TEST_F(RV32I_Handler_RI, SetLessThanImmediateUnsigned)
{
    NotImplemented();
}

TEST_F(RV32I_Handler_RI, ExcusiveOrImmediate)
{
    NotImplemented();
}

TEST_F(RV32I_Handler_RI, OrImmediate)
{
    NotImplemented();
}

TEST_F(RV32I_Handler_RI, AndImmediate)
{
    NotImplemented();
}

TEST_F(RV32I_Handler_RI, ShiftLeftLogicalImmediate)
{
    NotImplemented();
}

TEST_F(RV32I_Handler_RI, ShiftRightLogicalImmediate)
{
    NotImplemented();
}

TEST_F(RV32I_Handler_RI, ShiftRightArithmeticImmediate)
{
    NotImplemented();
}

} // namespace tests::rv32i
