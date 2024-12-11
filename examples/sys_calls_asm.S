// syscall definitions for sys_calls_main.c
// note: asm file should be last
// ../bin/build sys_calls sys_calls_main.c sys_calls_asm.S

#define DEFINE_SYS_CALL(num, name) .global name; name: li a7, num ; ecall; ret;

.text

DEFINE_SYS_CALL(10, sys_exit)
DEFINE_SYS_CALL(1024, sys_open)
DEFINE_SYS_CALL(57, sys_close)
DEFINE_SYS_CALL(63, sys_read)


DEFINE_SYS_CALL( 1, put_int)
DEFINE_SYS_CALL(11, put_char)
