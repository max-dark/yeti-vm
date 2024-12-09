// note: asm file should be last
// ../bin/build sys_calls sys_calls_main.c sys_calls_asm.S
// compile steps:
// ../bin/cc -c sys_calls_asm.S
// ../bin/cc -c sys_calls_main.c
// ../bin/cc sys_calls_main.o sys_calls_asm.o -o sys_calls.elf
// ../bin/objcopy -O binary sys_calls.elf sys_calls.bin

#include <stdint.h>

extern void sys_exit(void);
extern int32_t sys_open(const char* filename, uint32_t flags);
extern int32_t sys_read(uint32_t fd, void * buffer, uint32_t size);
extern void sys_close(uint32_t fd);

extern void put_int(int32_t num);
extern void put_char(char c);

#define ELEM_PER_LINE 5
#define NUM_ELEM 25

const char fileName[] = "input.txt";
int32_t buffer[NUM_ELEM];
int32_t input[NUM_ELEM];
int32_t result[NUM_ELEM];

void _start()
{

    int32_t fd = sys_open(fileName, 0);
    sys_read(fd, buffer, sizeof(buffer));
    sys_close(fd);

    for (int idx = 0; idx < NUM_ELEM;)
    {
        for (int col = 0; col < ELEM_PER_LINE; ++col)
        {
            put_int(buffer[idx]);
            put_char(' ');
            ++idx;
        }
        put_char('\n');
    }

    sys_exit();
}