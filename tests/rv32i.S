.text
.global _start

_start:
    lui a0, 123
    auipc a2, 123
    jal ra, lbl_jal
lbl_jal :
    # jalr al, zero, lbl_jalr # ???
lbl_jalr:
    ecall
    ebreak

    jal zero, _start