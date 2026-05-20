#define SYSCALL_OPEN  3
#define SYSCALL_CLOSE 4
#define SYSCALL_WRITE 5
#define SYSCALL_READ  2

static void sys_print(const char *s) {
    __asm__ volatile ("int $0x80" : : "a"(0), "b"(s));
}

static int sys_open(const char *path) {
    int fd;
    __asm__ volatile ("int $0x80" : "=a"(fd) : "a"(3), "b"(path));
    return fd;
}

static int sys_read(int fd, char *buf, unsigned int count) {
    int ret;
    __asm__ volatile ("int $0x80" : "=a"(ret) : "a"(2), "b"(fd), "c"(buf), "d"(count));
    return ret;
}

static int sys_close(int fd) {
    int ret;
    __asm__ volatile ("int $0x80" : "=a"(ret) : "a"(4), "b"(fd));
    return ret;
}

static void sys_exit() {
    __asm__ volatile ("int $0x80" : : "a"(1));
}

void _start(int argc, char **argv) {
    int fd = sys_open("hello.txt");
    if (fd < 0) {
        sys_print("open failed\n");
        sys_exit();
    }

    char buf[64];
    int n = sys_read(fd, buf, 63);
    if (n > 0) {
        buf[n] = 0;
        sys_print(buf);
    } else {
        sys_print("read failed\n");
    }

    sys_close(fd);
    sys_exit();
}
