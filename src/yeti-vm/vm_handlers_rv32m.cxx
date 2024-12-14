#include "vm_handlers_rv32m.hxx"

namespace vm::rv32m
{

bool register_rv32m_set(registry *r)
{
    bool ok =  r->register_handler<mul>();
    ok = ok && r->register_handler<mulh>();
    ok = ok && r->register_handler<mulhsu>();
    ok = ok && r->register_handler<mulhu>();
    ok = ok && r->register_handler<div>();
    ok = ok && r->register_handler<divu>();
    ok = ok && r->register_handler<rem>();
    ok = ok && r->register_handler<remu>();

    return ok;
}
} // namespace vm::rv32m
