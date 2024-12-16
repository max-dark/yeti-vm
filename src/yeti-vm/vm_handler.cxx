#include "vm_handler.hxx"
#include "vm_utility.hxx"

namespace vm
{

bool registry::register_handler(interface::ptr handler)
{
    auto& id = handler->get_id();
    auto [it, ok] = handlers.try_emplace(id, handler);

    ensure(ok, it->second->get_mnemonic());

    if (handler->get_func_a() != no_func_a)
        func_a.insert(handler->get_code_base());
    if (handler->get_func_b() != no_func_b)
        func_b.insert(handler->get_code_base() | (handler->get_func_a() << 8));

    return ok;
}

registry::handler_ptr registry::find_handler(const opcode::Decoder *code) const
{
    auto op = code->get_code();
    auto funcA = func_a.contains(op) ? code->get_func3() : no_func_a;
    auto funcB = func_b.contains(op | (code->get_func3() << 8)) ? code->get_func7() : no_func_b;
    InstructionId id{op, opcode::UNKNOWN, funcA, funcB};
    const auto handler = handlers.find(id);
    if (handler != handlers.end())
    {
        return handler->second.get();
    }

    return nullptr;
}
} // namespace vm
