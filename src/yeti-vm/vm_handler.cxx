#include "vm_handler.hxx"
#include "vm_utility.hxx"

namespace vm
{

bool HandlerRegistry::register_handler(HandlerInterface::ptr handler)
{
    auto& id = handler->getId();
    auto [it, ok] = handlers.try_emplace(id, handler);

    if (!ok)
        return false;

    if (handler->getFuncA() != NoFuncA)
        func_a.insert(handler->getGroupId());
    if (handler->getFuncB() != NoFuncB)
        func_b.insert(handler->getGroupId() | (handler->getFuncA() << 8));

    return ok;
}

HandlerRegistry::handler_ptr HandlerRegistry::find_handler(const opcode::Decoder *code) const
{
    auto op = code->get_code();
    auto funcA = func_a.contains(op) ? code->get_func3() : NoFuncA;
    auto funcB = func_b.contains(op | (code->get_func3() << 8)) ? code->get_func7() : NoFuncB;
    InstructionId id{op, opcode::UNKNOWN, funcA, funcB};
    const auto handler = handlers.find(id);
    if (handler != handlers.end())
    {
        return handler->second.get();
    }

    return nullptr;
}
} // namespace vm
