/*
 *  link script for test configuration
 */
ENTRY(rvtest_entry_point);

MEMORY
{
    /* program code region */
    ROM(rx)     : ORIGIN = 0M,  LENGTH = 4M
    /* data region */
    RAM(rw!x)   : ORIGIN = 4M,  LENGTH = 4M
    /* "device" region */
    DEV(rw!x)   : ORIGIN = 8M,  LENGTH = 4M
}

__ram_start = ORIGIN(RAM);
__ram_end   = ORIGIN(RAM) + LENGTH(RAM);

__dev_start = ORIGIN(DEV);
__dev_end   = ORIGIN(DEV) + LENGTH(DEV);

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