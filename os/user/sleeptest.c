#include <stdio.h>

static int sys_sleep(unsigned int ms) {
    int ret;
    __asm__ volatile (
        "int $0x80"
        : "=a"(ret)
        : "a"(7), "b"(ms)
    );
    return ret;
}

int main() {
    printf("sleeping 2 seconds...\n");
    sys_sleep(2000);
    printf("done!\n");
    return 0;
}
