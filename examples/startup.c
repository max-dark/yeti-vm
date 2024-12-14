#include <stdint.h>

void _entry() __attribute__((naked, noreturn));
void _sys_exit() __attribute__((naked, noreturn));
void _init_rt() __attribute__((noreturn));

void _start();

void _entry()
{
    asm volatile (
            ".option push;\n"
            ".option norelax;\n"
            "la gp, __global_pointer$;\n"
            ".option pop;\n"
            "la sp, __ram_end;\n"
            "j _init_rt;\n"
            );
}

void _init_rt()
{
    _start();
    _sys_exit();
}

void _sys_exit()
{
    asm volatile (
            "li a7, 10;\n"
            "ecall;\n"
            );
}