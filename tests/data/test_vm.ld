/*
 *  link script for test configuration
 *
 *  based on https://github.com/five-embeddev/riscv-isa-sim/tree/master/arch_test_target/spike
 */
ENTRY(_entry);

MEMORY
{
    ROM(rx)     : ORIGIN = 0M,  LENGTH = 4M
    RAM(rw!x)   : ORIGIN = 4M,  LENGTH = 4M
}

__ram_start = ORIGIN(RAM);
__ram_end   = ORIGIN(RAM) + LENGTH(RAM);

SECTIONS
{
    .text : {
        *(.text.init)
        . = ALIGN(4);
        *(.text)
    } > ROM
    .rodata : {
        . = ALIGN(4);
        *(.rodata)
    } > ROM
    .data : {
        . = ALIGN(4);
        PROVIDE(__global_pointer$ = . + 0x800);
        *(.data)
    } > RAM AT > ROM
    .data.string : {
        *(.data.string)
    } > RAM AT > ROM
    .bss  : {
        . = ALIGN(4);
        *(.bss )
    } > RAM
}