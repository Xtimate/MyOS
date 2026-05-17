void _start() {
    // sys_print via int 0x80
    asm volatile (
        "mov $0, %%eax\n"
        "mov $1f, %%ebx\n"
        "int $0x80\n"
        "1: .asciz \"Hello from user space!\\n\""
        ::: "eax", "ebx"
    );

    // sys_exit
    asm volatile (
        "mov $1, %%eax\n"
        "int $0x80\n"
        ::: "eax"
    );
}
