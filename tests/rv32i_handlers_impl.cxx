#include "rv32i_handlers.hxx"
#include <yeti-vm/vm_handlers_rv32i.hxx>

namespace tests::rv32i
{
using vm::opcode::Encoder;
using vm::opcode::Decoder;
using namespace vm::rv32i;

using ::testing::_;
using ::testing::Args;

using Format = vm::opcode::BaseFormat;

class RV32I_Handler_Impl: public RV32I_Handler {};

TEST_F(RV32I_Handler_Impl, none)
{
}

TEST_F(RV32I_Handler_Impl, LoadUpperImmediate)
{
    auto impl = create<lui>();

    ASSERT_EQ(impl->get_code_base(), Enum::LUI);
    ASSERT_EQ(impl->get_type(), Format::U_TYPE);

    EXPECT_CALL(mock(), set_register(_, _))
        .Times(vm::register_count);
    for (uint8_t r_id = 0; r_id < vm::register_count; ++ r_id)
    {
        Code imm = r_id << 12u;
        Code code = Encoder::u_type(Enum::LUI, r_id, imm);
        Decoder parser{code};
        ASSERT_EQ(parser.get_rd(), r_id);
        ASSERT_EQ(parser.decode_u_u(), imm);
        ASSERT_EQ(parser.get_code(), impl->get_code_base());

        EXPECT_CALL(mock(), set_register(r_id, imm));
        impl->exec(vm(), &parser);
    }
}

} // namespace tests::rv32i
