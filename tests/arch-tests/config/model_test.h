// config for https://github.com/riscv-non-isa/riscv-arch-test/
// docs: https://github.com/riscv-non-isa/riscv-arch-test/releases

#ifndef YETI_VM_MODEL_H
#define YETI_VM_MODEL_H

// Supports rv32
#define XLEN 32
#define ALIGNMENT 2

#define TEST_CASE_1

// startup code
#define RVMODEL_BOOT \
    RVMODEL_IO_INIT  \

// stop code
#define RVMODEL_HALT \
    li a7, 10;       \
    ecall;           \
    .global _assert_failed; \
    _assert_failed: ebreak; \

#define RVMODEL_DATA_BEGIN \
    .align 4;              \
    .global begin_signature; \
    begin_signature:

#define RVMODEL_DATA_END \
    .align 4;            \
    .global end_signature; \
    end_signature:

#define RVMODEL_IO_INIT
#define RVMODEL_IO_WRITE_STR(_R, _STR)
#define RVMODEL_IO_CHECK()

// asserts: testreg, destreg, correctval
// generic purpose registers:
#define RVMODEL_IO_ASSERT_GPR_EQ(_S, _R, _I) \
    LI(_S, _I);                               \
    bne _S, _R, _assert_failed;              \

// float32 registers
#define RVMODEL_IO_ASSERT_SFPR_EQ(_F, _R, _I)
// float64 registers
#define RVMODEL_IO_ASSERT_DFPR_EQ(_D, _R, _I)

// machine-mode interrupts
// use default behavior - end test
// TODO: learn about it
//#define RVMODEL_SET_MSW_INT
//#define RVMODEL_CLEAR_MSW_INT
//#define RVMODEL_CLEAR_MTIMER_INT
//#define RVMODEL_CLEAR_MEXT_INT

#endif // YETI_VM_MODEL_H
