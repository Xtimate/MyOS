static const char msg[] = "Hello from user space!\n";

void _start() {
    __asm__ volatile (
        "mov $0, %%eax\n"
        "mov %0, %%ebx\n"
        "int $0x80\n"
        :
        : "r"(msg)
        : "eax", "ebx"
    );

    __asm__ volatile (
        "mov $1, %%eax\n"
        "xor %%ebx, %%ebx\n"
        "int $0x80\n"
        ::: "eax", "ebx"
    );

    while (1) {}
}
