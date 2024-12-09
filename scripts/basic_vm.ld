ENTRY(_start);

MEMORY
{
    ROM(rx)     : ORIGIN = 0M,  LENGTH = 4M
    RAM(rw!x)   : ORIGIN = 4M,  LENGTH = 4M
}

SECTIONS
{
    .text : {
        *(.text)
    } > ROM
    .rodata : {
        *(.rodata)
    } > ROM
    .data : {
        *(.data)
    } > ROM AT > RAM
    .bss  : {
        *(.bss )
    } > RAM
}
