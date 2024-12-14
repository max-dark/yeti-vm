.text
.global _entry

_entry:
    .option push
    la gp, __global_pointer$
    .option pop
    la sp, __ram_end
    tail _start