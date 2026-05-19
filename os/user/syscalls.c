#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <stdint.h>

// raw syscall helpers
static int sys_raw_write(int fd, const void *buf, int count) {
    int ret;
    __asm__ volatile ("int $0x80" : "=a"(ret) : "a"(5), "b"(fd), "c"(buf), "d"(count));
    return ret;
}

static int sys_raw_read(int fd, void *buf, int count) {
    int ret;
    __asm__ volatile ("int $0x80" : "=a"(ret) : "a"(2), "b"(fd), "c"(buf), "d"(count));
    return ret;
}

static int sys_raw_open(const char *path) {
    int ret;
    __asm__ volatile ("int $0x80" : "=a"(ret) : "a"(3), "b"(path));
    return ret;
}

static int sys_raw_close(int fd) {
    int ret;
    __asm__ volatile ("int $0x80" : "=a"(ret) : "a"(4), "b"(fd));
    return ret;
}

static unsigned int sys_raw_brk(unsigned int addr) {
    unsigned int ret;
    __asm__ volatile ("int $0x80" : "=a"(ret) : "a"(6), "b"(addr));
    return ret;
}

// newlib stubs

void _exit(int status) {
    __asm__ volatile ("int $0x80" : : "a"(1));
    while (1) {}
}

int _write(int fd, char *buf, int count) {
    return sys_raw_write(fd, buf, count);
}

int _read(int fd, char *buf, int count) {
    return sys_raw_read(fd, buf, count);
}

int _open(const char *path, int flags, ...) {
    return sys_raw_open(path);
}

int _close(int fd) {
    return sys_raw_close(fd);
}

static unsigned int heap_end = 0;

void *_sbrk(int incr) {
    if (heap_end == 0)
        heap_end = sys_raw_brk(0);

    unsigned int old = heap_end;
    heap_end = sys_raw_brk(heap_end + incr);
    return (void *)old;
}

int _fstat(int fd, struct stat *st) {
    st->st_mode = S_IFCHR;
    return 0;
}

int _isatty(int fd) {
    return (fd <= 2) ? 1 : 0;
}

int _lseek(int fd, int offset, int whence) {
    return -1;
}

int _getpid() {
    return 1;
}

int _kill(int pid, int sig) {
    errno = EINVAL;
    return -1;
}

int close(int fd) { return _close(fd); }
int lseek(int fd, int offset, int whence) { return _lseek(fd, offset, whence); }
int read(int fd, char *buf, int count) { return _read(fd, buf, count); }
int write(int fd, char *buf, int count) { return _write(fd, buf, count); }
void *sbrk(int incr) { return _sbrk(incr); }
int fstat(int fd, struct stat *st) { return _fstat(fd, st); }
int isatty(int fd) { return _isatty(fd); }
int kill(int pid, int sig) { errno = EINVAL; return -1; }
int getpid() { return 1; }
