.text
.global _start

_start:
    lui a0, 123
    auipc a2, 123
    jal ra, lbl_jal
lbl_jal :
    # jalr al, zero, lbl_jalr # ???
    jalr ra, 4(x0)
lbl_jalr:
    beq t4, x6, lbl_branch
    bne t4, x6, lbl_branch
    blt t4, x6, lbl_branch
    bge t4, x6, lbl_branch
    bltu t4, x6, lbl_branch
    bgeu t4, x6, lbl_branch
lbl_branch:
    ecall
    ebreak

    jal zero, _start