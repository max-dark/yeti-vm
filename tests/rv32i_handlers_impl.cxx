#include "rv32i_handlers.hxx"
#include <yeti-vm/vm_handlers_rv32i.hxx>

namespace tests::rv32i
{
using namespace vm::rv32i;

using RegId = vm::register_no;
using RegAlias = vm::RegAlias;
using Address = vm::vm_interface::address_t;
using Offset = vm::vm_interface::offset_t;

using ::testing::_;
using ::testing::Args;
using ::testing::Return;

class RV32I_Handler_Impl: public RV32I_Handler {};

TEST_F(RV32I_Handler_Impl, LoadUpperImmediate)
{
    MockVM mock;
    auto impl = create<lui>();

    auto& id = impl->get_id();
    // group / encoding / no extensions
    ASSERT_TRUE(id.equal(make_id(GroupId::LUI, Format::U_TYPE)));

    for (uint8_t r_id = 0; r_id < vm::register_count; ++ r_id)
    {
        Code imm = r_id << 12u;
        Code code = Encoder::u_type(GroupId::LUI, r_id, imm);
        Decoder parser{code};
        ASSERT_EQ(parser.get_rd(), r_id);
        ASSERT_EQ(parser.decode_u_u(), imm);
        ASSERT_EQ(parser.get_code(), impl->get_code_base());

        EXPECT_CALL(mock, set_register(r_id, imm));
        impl->exec(&mock, &parser);
    }
}

TEST_F(RV32I_Handler_Impl, AddUpperImmediateToPC)
{
    MockVM mock;
    auto impl = create<auipc>();

    auto& id = impl->get_id();
    // group / encoding / no extensions
    ASSERT_TRUE(id.equal(make_id(GroupId::AUIPC, Format::U_TYPE)));

    for (uint8_t r_id = 0; r_id < vm::register_count; ++ r_id)
    {
        Code imm = r_id << 12u; // TODO: test with MSB = 1
        Code code = Encoder::u_type(impl->get_code_base(), r_id, imm);
        Decoder parser{code};
        ASSERT_EQ(parser.get_rd(), r_id);
        ASSERT_EQ(parser.decode_u_u(), imm);
        ASSERT_EQ(parser.get_code(), impl->get_code_base());

        EXPECT_CALL(mock, get_pc())
            .WillRepeatedly(Return(r_id * r_id));
        EXPECT_CALL(mock, set_register(r_id, imm + r_id * r_id));
        impl->exec(&mock, &parser);
    }

}

TEST_F(RV32I_Handler_Impl, JumpAndLink)
{
    MockVM mock;
    // jal instruction ...
    auto impl = create<jal>();

    auto& id = impl->get_id();
    // group / encoding / no extensions
    ASSERT_TRUE(id.equal(make_id(GroupId::JAL, Format::J_TYPE)));

    // immediate encodes signed offset(x2, imm[0] ignored) relative to PC value
    // addr = pc + imm
    // generates exception if <addr> not aligned to four bytes
    // ok = <addr> % 4 !=0
    // saves next instruction address in dest
    // rd = pc + 4

    RegId regId[] = { RegAlias::zero, RegAlias::ra, RegAlias::t0 };
    Offset offset[] = { -8, -4, 0, +4, +8 };
    Address pc_min = 0;
    Address pc_max = 32;

    using bits = vm::bit_tools::bits<Offset>;
    for (auto pc = pc_min; pc <= pc_max; ++pc)
    {
        Address ptr = pc * 4;
        for(auto o: offset)
        {
            auto imm = bits::to_unsigned(o);
            for (auto rd: regId)
            {
                Decoder parser{ Encoder::j_type(GroupId::JAL, rd, imm) };
                EXPECT_CALL(mock, get_pc())
                    .WillRepeatedly(Return(ptr));
                EXPECT_CALL(mock, set_register(rd, ptr + 4));
                EXPECT_CALL(mock, jump_to(o));

                impl->exec(&mock, &parser);
            }
        }
    }
}

TEST_F(RV32I_Handler_Impl, JumpAndLinkRegister)
{
    MockVM mock;
    // jalr instruction ...
    auto impl = create<jalr>();

    auto &id = impl->get_id();
    // group / encoding / have a-ext[0]
    ASSERT_TRUE(id.equal(make_id(GroupId::JALR, Format::I_TYPE, 0)));

    // immediate encodes signed offset relative to RS1 value
    // addr = rs1 + imm
    // generates exception if <addr> not aligned to four bytes
    // ok = <addr> % 4 !=0
    // saves next instruction address in dest
    // rd = pc + 4

    RegId regId[] = { RegAlias::zero, RegAlias::ra, RegAlias::t0 };
    Offset offset[] = { -8, -4, 0, +4, +8 };
    Address pc_min = 0;
    Address pc_max = 32;

    using bits = vm::bit_tools::bits<Offset>;
    RegId rs1 = 0;
    for (auto pc = pc_min; pc <= pc_max; ++pc)
    {
        Address ptr = pc * 4;
        for (auto o: offset)
        {
            auto imm = bits::to_unsigned(o);
            for (auto rd: regId)
            {
                Decoder parser{ Encoder::i_type(GroupId::JALR, rd, rs1, imm, 0) };
                auto rs1_value = ptr * rs1;
                EXPECT_CALL(mock, get_register(rs1))
                    .WillRepeatedly(Return(rs1_value));
                EXPECT_CALL(mock, get_pc())
                        .WillRepeatedly(Return(ptr));
                EXPECT_CALL(mock, set_register(rd, ptr + 4));
                EXPECT_CALL(mock, jump_abs(rs1_value + o));

                impl->exec(&mock, &parser);

                rs1 = (rs1 + 1) % vm::register_count;
            }
        }
    }
}

} // namespace tests::rv32i
