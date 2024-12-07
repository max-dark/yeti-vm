#include "vm_syscall.hxx"
#include <vm_utility.hxx>

namespace vm
{
syscall_interface::~syscall_interface() = default;

bool syscall_registry::register_handler(syscall_interface::ptr handler)
{
    auto [it, ok] = handlers.try_emplace(handler->get_id(), handler);
    ensure(ok, it->second->get_name());
    return ok;
}

syscall_registry::handler_ptr syscall_registry::find_handler(const vm_interface *vm) const
{
    auto handler = handlers.find(vm->get_register(vm::RegAlias::a7));
    if (handler != handlers.end())
    {
        return handler->second.get();
    }
    return nullptr;
}
}// namespace vm
