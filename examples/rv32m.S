// test for 'm' extension(math)
// compile:
// ../bin/build rv32m rv32m.S

.text
.global _start

_start:
    mul     a0, a1, a2
    mulh    a0, a1, a2
    mulhsu  a0, a1, a2
    mulhu   a0, a1, a2

    div  a0, a1, a2
    divu a0, a1, a2
    rem  a0, a1, a2
    remu a0, a1, a2

    jal zero, _start