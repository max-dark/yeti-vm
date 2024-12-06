#include "vm_handlers_rv32i.hxx"

namespace vm::rv32i
{
namespace // static
{
void add_branch(registry *r)
{
    r->register_handler<beq>();
    r->register_handler<bne>();
    r->register_handler<blt>();
    r->register_handler<bge>();
    r->register_handler<bltu>();
    r->register_handler<bgeu>();
}

void add_load(registry *r)
{
    r->register_handler<lb>();
    r->register_handler<lh>();
    r->register_handler<lw>();
    r->register_handler<lbu>();
    r->register_handler<lhu>();
}

void add_store(registry *r)
{
    r->register_handler<sb>();
    r->register_handler<sh>();
    r->register_handler<sw>();
}

void add_int_imm(registry *r)
{
    r->register_handler<addi>();
    r->register_handler<sli>();
    r->register_handler<sliu>();
    r->register_handler<xori>();
    r->register_handler<ori>();
    r->register_handler<andi>();
}

void add_shift_imm(registry *r)
{
    r->register_handler<slli>();
    r->register_handler<srli>();
    r->register_handler<srai>();
}

void add_int(registry *r)
{
    r->register_handler<add_r>();
    r->register_handler<sub_r>();
    r->register_handler<sll_r>();
    r->register_handler<sll_r>();
    r->register_handler<slt_r>();
    r->register_handler<sltu_r>();
    r->register_handler<xor_r>();
    r->register_handler<srl_r>();
    r->register_handler<sra_r>();
    r->register_handler<or_r>();
    r->register_handler<and_r>();
}

void add_misc(registry *r)
{
    r->register_handler<fence>();
    r->register_handler<fence_i>();
}

void add_system(registry *r)
{
    r->register_handler<env_call>();
    r->register_handler<csrrw>();
    r->register_handler<csrrs>();
    r->register_handler<csrrc>();
    r->register_handler<csrrwi>();
    r->register_handler<csrrsi>();
    r->register_handler<csrrci>();
}

} // namespace // static

void register_rv32i_set(registry *r)
{
    r->register_handler<lui>();
    r->register_handler<auipc>();
    r->register_handler<jal>();
    r->register_handler<jalr>();

    add_branch(r);
    add_load(r);
    add_store(r);
    add_int_imm(r);
    add_shift_imm(r);
    add_int(r);
    add_misc(r);
    add_system(r);
}
} // namespace vm::rv32i
