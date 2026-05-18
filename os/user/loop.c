static const char msg[] = "loop\n";

void _start() {
    while (1) {
        register const char *str asm("ebx") = msg;
        __asm__ volatile (
            "mov $0, %%eax\n"
            "int $0x80\n"
            : : "r"(str) : "eax"
        );
        for (volatile int i = 0; i < 50000000; i++);
    }
}
