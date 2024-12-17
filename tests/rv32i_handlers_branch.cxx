#include "rv32i_handlers.hxx"
#include <yeti-vm/vm_handlers_rv32i.hxx>

namespace tests::rv32i
{
using namespace vm::rv32i;

using ::testing::_;
using ::testing::Args;
using ::testing::Return;

class RV32I_Handler_Branch : public RV32I_Handler {};

} // namespace tests::rv32i
