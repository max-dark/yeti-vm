// note: asm file should be last
// ../bin/build sys_calls sys_calls_main.c sys_calls_asm.S
// compile steps:
// ../bin/cc -c sys_calls_asm.S
// ../bin/cc -c sys_calls_main.c
// ../bin/cc sys_calls_main.o sys_calls_asm.o -o sys_calls.elf
// ../bin/objcopy -O binary sys_calls.elf sys_calls.bin

#include <stdint.h>

[[noreturn]]
void sys_exit(void);
int32_t sys_open(const char* filename, uint32_t flags);
int32_t sys_read(uint32_t fd, void * buffer, uint32_t size);
void sys_close(uint32_t fd);

void put_int(int32_t num);
void put_char(char c);

#define ELEM_PER_LINE 5
#define NUM_LINES 5
#define NUM_ELEM (ELEM_PER_LINE * NUM_LINES)

const char fileName[] = "input.txt";
int32_t buffer[NUM_ELEM];
int32_t input[NUM_LINES][NUM_ELEM];
int32_t result[NUM_LINES][NUM_ELEM];

void app_main();

[[noreturn]]
void _start()
{
    app_main();
    sys_exit();
}

void app_main()
{
    int fd = sys_open(fileName, 0);
    sys_read(fd, buffer, sizeof(buffer));
    sys_close(fd);

    int idx = 0;
    for (int row = 0; row < NUM_LINES; ++row)
    {
        for (int col = 0; col < ELEM_PER_LINE; ++col)
        {
            input[row][col] = buffer[idx];
            ++idx;
        }
    }

    int k = 0;
    int max_sum = ~0;
    for (int row = 0; row < NUM_LINES; ++row)
    {
        int current_sum = 0;
        for (int col = 0; col < ELEM_PER_LINE; ++col)
        {
            current_sum += input[row][col];
        }
        if (current_sum > max_sum)
        {
            k = row;
        }
    }

    for (int row = 0; row < NUM_LINES; ++row)
    {
        for (int col = 0; col < ELEM_PER_LINE; ++col)
        {
            result[row][col] = input[row][col] / input[k][col];
        }
    }

    for (int row = 0; row < NUM_LINES; ++row)
    {
        for (int col = 0; col < ELEM_PER_LINE; ++col)
        {
            put_int(result[row][col]);
            put_char(' ');
        }
        put_char('\n');
    }
}
