#include "vm_handlers_rv32i.hxx"

namespace vm::rv32i
{
namespace // static
{
void add_branch(registry *r, bool& ok)
{
    ok = ok && r->register_handler<beq>();
    ok = ok && r->register_handler<bne>();
    ok = ok && r->register_handler<blt>();
    ok = ok && r->register_handler<bge>();
    ok = ok && r->register_handler<bltu>();
    ok = ok && r->register_handler<bgeu>();
}

void add_load(registry *r, bool& ok)
{
    ok = ok && r->register_handler<lb>();
    ok = ok && r->register_handler<lh>();
    ok = ok && r->register_handler<lw>();
    ok = ok && r->register_handler<lbu>();
    ok = ok && r->register_handler<lhu>();
}

void add_store(registry *r, bool& ok)
{
    ok = ok && r->register_handler<sb>();
    ok = ok && r->register_handler<sh>();
    ok = ok && r->register_handler<sw>();
}

void add_int_imm(registry *r, bool& ok)
{
    ok = ok && r->register_handler<addi>();
    ok = ok && r->register_handler<slti>();
    ok = ok && r->register_handler<sltiu>();
    ok = ok && r->register_handler<xori>();
    ok = ok && r->register_handler<ori>();
    ok = ok && r->register_handler<andi>();
}

void add_shift_imm(registry *r, bool& ok)
{
    ok = ok && r->register_handler<slli>();
    ok = ok && r->register_handler<srli>();
    ok = ok && r->register_handler<srai>();
}

void add_int(registry *r, bool& ok)
{
    ok = ok && r->register_handler<add_r>();
    ok = ok && r->register_handler<sub_r>();
    ok = ok && r->register_handler<sll_r>();
    ok = ok && r->register_handler<slt_r>();
    ok = ok && r->register_handler<sltu_r>();
    ok = ok && r->register_handler<xor_r>();
    ok = ok && r->register_handler<srl_r>();
    ok = ok && r->register_handler<sra_r>();
    ok = ok && r->register_handler<or_r>();
    ok = ok && r->register_handler<and_r>();
}

void add_misc(registry *r, bool& ok)
{
    ok = ok && r->register_handler<fence>();
    ok = ok && r->register_handler<fence_i>();
}

void add_system(registry *r, bool& ok)
{
    ok = ok && r->register_handler<env_call>();
    ok = ok && r->register_handler<csrrw>();
    ok = ok && r->register_handler<csrrs>();
    ok = ok && r->register_handler<csrrc>();
    ok = ok && r->register_handler<csrrwi>();
    ok = ok && r->register_handler<csrrsi>();
    ok = ok && r->register_handler<csrrci>();
}

} // namespace // static

bool register_rv32i_set(registry *r)
{
    bool ok = true;

    add_branch(r, ok);
    add_load(r, ok);
    add_store(r, ok);
    add_int_imm(r, ok);
    add_shift_imm(r, ok);
    add_int(r, ok);
    add_misc(r, ok);
    add_system(r, ok);

    ok = ok && r->register_handler<lui>();
    ok = ok && r->register_handler<auipc>();
    ok = ok && r->register_handler<jal>();
    ok = ok && r->register_handler<jalr>();

    return ok;
}
} // namespace vm::rv32i
