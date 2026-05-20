static char buf[2] = {0, 0};
static const char prompt[] = "enter a key: \n";
static const char got[] = "you pressed: ";
static const char newline[] = "\n";

void _start(int argc, char **argv) {
    while (1) {
        register const char *str asm("ebx") = prompt;
        __asm__ volatile ("mov $0, %%eax\n int $0x80\n" : : "r"(str) : "eax");

        unsigned int bytes = 0;
        while (bytes == 0) {
            register char *b asm("ebx") = buf;
            register unsigned int len asm("ecx") = 1;
            __asm__ volatile (
                "mov $2, %%eax\n"
                "int $0x80\n"
                "mov %%eax, %0\n"
                : "=r"(bytes) : "r"(b), "r"(len) : "eax"
            );
        }

        str = got;
        __asm__ volatile ("mov $0, %%eax\n int $0x80\n" : : "r"(str) : "eax");

        str = buf;
        __asm__ volatile ("mov $0, %%eax\n int $0x80\n" : : "r"(str) : "eax");

        str = newline;
        __asm__ volatile ("mov $0, %%eax\n int $0x80\n" : : "r"(str) : "eax");
    }
}
