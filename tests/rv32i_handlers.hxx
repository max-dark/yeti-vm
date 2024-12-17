#include "rv32i_mocks.hxx"

#include <yeti-vm/vm_opcode.hxx>
#include <yeti-vm/vm_handler.hxx>
#include <concepts>

namespace tests::rv32i
{
using vm::register_t;
using vm::register_no;

using Enum = vm::opcode::OpcodeType;
using Type = vm::opcode::Decoder;
using Code = vm::opcode::opcode_t;


template<class Handler>
concept Implementation = std::is_base_of_v<vm::interface, Handler>;
using HandlerPtr = std::unique_ptr<vm::interface>;

class RV32I_Handler : public ::testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
        ptr.reset();
    }

    template<Implementation Type>
    [[nodiscard]]
    vm::interface *create()
    {
        ptr = std::make_unique<Type>();
        return ptr.get();
    }

    [[nodiscard]]
    vm::interface *get() const
    {
        return ptr.get();
    }

    static vm::vm_interface& vm() {
        static thread_local MockVM mockVm;
        return mockVm;
    }

    static Type* code(Code code)
    {
        static thread_local Type current;
        current.code = code;
        return &current;
    }

private:
    HandlerPtr ptr;
};

} // namespace tests::rv32i
