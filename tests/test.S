.text
.global _start

_start:
    la a0, 123456
    call some
ret

some:
    addi a0, a0, 0
ret
