#include "include/input.h"

#define INPUT_BUFFER_SIZE 256

static char buffer[INPUT_BUFFER_SIZE];
static int read_pos = 0;
static int write_pos = 0;
static int count = 0;

void input_init() {
    read_pos = 0;
    write_pos = 0;
    count = 0;
}

void input_putchar(char c) {
    if (count >= INPUT_BUFFER_SIZE) return;
    buffer[write_pos] = c;
    write_pos = (write_pos + 1) % INPUT_BUFFER_SIZE;
    count++;
}

int input_getchar() {
    if (count == 0) return -1;
    char c = buffer[read_pos];
    read_pos = (read_pos + 1) % INPUT_BUFFER_SIZE;
    count--;
    return c;
}

int input_available() {
    return count;
}
