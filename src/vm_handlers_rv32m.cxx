#include "vm_handlers_rv32m.hxx"

namespace vm::rv32m
{

void register_rv32m_set(registry *r)
{
    r->register_handler<mul>();
    r->register_handler<mulh>();
    r->register_handler<mulhsu>();
    r->register_handler<mulhu>();
    r->register_handler<div>();
    r->register_handler<divu>();
    r->register_handler<rem>();
    r->register_handler<remu>();
}
} // namespace vm::rv32m
