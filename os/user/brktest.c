#define SYSCALL_BRK 6

static void sys_print(const char *s) {
    __asm__ volatile ("int $0x80" : : "a"(0), "b"(s));
}

static unsigned int sys_brk(unsigned int addr) {
    unsigned int ret;
    __asm__ volatile ("int $0x80" : "=a"(ret) : "a"(6), "b"(addr));
    return ret;
}

static void sys_exit() {
    __asm__ volatile ("int $0x80" : : "a"(1));
}

static void print_num(unsigned int n) {
    char buf[12];
    int i = 10;
    buf[11] = 0;
    if (n == 0) { sys_print("0"); return; }
    while (n && i >= 0) {
        buf[i--] = '0' + (n % 10);
        n /= 10;
    }
    sys_print(&buf[i + 1]);
}

void _start(int argc, char **argv) {
    // get current brk
    unsigned int cur = sys_brk(0);
    sys_print("initial brk: ");
    print_num(cur);
    sys_print("\n");

    // grow by 4096
    unsigned int new_brk = sys_brk(cur + 4096);
    sys_print("new brk: ");
    print_num(new_brk);
    sys_print("\n");

    // write to the new memory
    char *ptr = (char *)cur;
    ptr[0] = 'H';
    ptr[1] = 'i';
    ptr[2] = '\n';
    ptr[3] = 0;

    sys_print("wrote to heap: ");
    sys_print(ptr);

    sys_exit();
}
