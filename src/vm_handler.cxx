#include "vm_handler.hxx"

namespace vm
{

void registry::register_handler(interface::ptr handler)
{
    auto& id = handler->get_id();
    auto& type = handlers[id.code];
    type[id] = std::move(handler);
}

registry::handler_ptr registry::find_handler(const opcode::OpcodeBase *code) const
{
    auto op = code->get_code();
    const auto type = handlers.find(op);
    if (type != handlers.end())
    {
        InstructionId id;
        id.code = op;
        id.format = opcode::UNKNOWN;
        id.funcA = opcode::have_ext_a(op) ? code->get_func3() : no_func_a;
        id.funcB = opcode::have_ext_b(op) ? code->get_func7() : no_func_a;

        auto& tmp = type->second;
        auto handler = tmp.find(id);
        if (handler != tmp.end())
            return handler->second.get();
    }

    return nullptr;
}
} // namespace vm
