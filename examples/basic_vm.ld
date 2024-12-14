/*
    link script for default configuration
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
        *(.text)
    } > ROM
    .rodata : {
        *(.rodata)
    } > ROM
    .data : {
        PROVIDE(__global_pointer$ = . + 0x800);
        *(.data)
    } > RAM AT > ROM
    .bss  : {
        *(.bss )
    } > RAM
}