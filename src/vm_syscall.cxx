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

syscall_registry::handler_ptr syscall_registry::find_handler(syscall_registry::syscall_id id) const
{
    auto handler = handlers.find(id);
    if (handler != handlers.end())
    {
        return handler->second.get();
    }
    return nullptr;
}

syscall_registry::syscall_id syscall_registry::get_syscall_id(const vm_interface *vm) const
{
    return vm->get_register(RegAlias::a7);
}
}// namespace vm
