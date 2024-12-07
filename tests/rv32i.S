// test for RV32I

.text
.global _start

_start:
    lui a0, 123
    auipc a2, 123
    jal ra, lbl_jal
lbl_jal :
    jalr ra, 4(x0)
lbl_jalr:
    beq t4, x6, lbl_branch
    bne t4, x6, lbl_branch
    blt t4, x6, lbl_branch
    bge t4, x6, lbl_branch
    bltu t4, x6, lbl_branch
    bgeu t4, x6, lbl_branch
lbl_branch:
    // load
    lb t1, 0(zero)
    lh t1, 0(zero)
    lw t1, 0(zero)
    lbu t1, 0(zero)
    lhu t1, 0(zero)
    // store
    sb t1, 0(zero)
    sh t1, 0(zero)
    sw t1, 0(zero)

    andi t1, t2, 0
    slti t1, t2, 0
    sltiu t1, t2, 0
    xori t1, t2, 0
    ori  t1, t2, 0
    andi t1, t2, 0

    slli t1, t2, 0
    srli t1, t2, 0
    srai t1, t2, 0

    add t1, t2, t3
    sub t1, t2, t3
    sll t1, t2, t3
    slt t1, t2, t3
    sltu t1, t2, t3
    xor t1, t2, t3
    srl t1, t2, t3
    sra t1, t2, t3
    or  t1, t2, t3
    and t1, t2, t3

    // env
    ecall
    ebreak

    fence iorw, iorw
    // fence.i // extension `zifencei' required

    // extension `zicsr' required
    // csrrw t1, time, t2
    // csrrs t1, time, t2
    // csrrc t1, time, t2

    // csrrwi t1, time, 0
    // csrrsi t1, time, 0
    // csrrci t1, time, 0

    jal zero, _start
