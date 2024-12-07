#include "vm_handler.hxx"
#include <vm_utility.hxx>

namespace vm
{

bool registry::register_handler(interface::ptr handler)
{
    auto& id = handler->get_id();
    auto [it, ok] = handlers.try_emplace(id, handler);

    ensure(ok, it->second->get_mnemonic());

    return ok;
}

registry::handler_ptr registry::find_handler(const opcode::OpcodeBase *code) const
{
    auto op = code->get_code();
    auto funcA = opcode::have_ext_a(op) ? code->get_func3() : no_func_a;
    auto funcB = opcode::have_ext_b(op) ? code->get_func7() : no_func_b;
    InstructionId id{op, opcode::UNKNOWN, funcA, funcB};
    const auto handler = handlers.find(id);
    if (handler != handlers.end())
    {
        return handler->second.get();
    }

    return nullptr;
}
} // namespace vm
