#include "include/syscall.h"
#include "include/idt.h"
#include "include/kstring.h"
#include "include/paging.h"
#include "include/process.h"
#include "include/vga.h"
#include "include/process.h"
#include "include/input.h"
#include "include/fs.h"
#include "include/timer.h"

extern void isr128();
extern void kernel_thread_start(unsigned int new_esp);

static void outb(unsigned short port, unsigned char val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static void sys_print(struct registers *r) {
    char *str = (char *)r->ebx;
    vga_print(str);
}

static void sys_exit(struct registers *r) {
    vga_print("\n[Process exited]\n");
    if (current_process) {
        process_exit(current_process);
        current_process = 0;
    }
    outb(0x20, 0x20);
    __asm__ volatile ("sti");
    while (1) { __asm__ volatile ("hlt"); }
}

static void sys_file_read(struct registers *r) {
    int fd = (int)r->ebx;
    char *buf = (char *)r->ecx;
    unsigned int count = r->edx;

    if (!current_process || fd < 3 || fd >= MAX_FDS || !current_process->fds[fd].used) {
        r->eax = -1;
        return;
    }

    fd_t *f = &current_process->fds[fd];
    unsigned int remaining = f->size - f->offset;
    if (count > remaining) count = remaining;

    kmemcpy(buf, f->data + f->offset, count);
    f->offset += count;
    r->eax = count;
}

static void sys_read(struct registers *r) {
    int fd = (int)r->ebx;

    if (fd >= 3) {
        sys_file_read(r);
        return;
    }

    char *buf = (char *)r->ebx;
    unsigned int max = r->ecx;

    if (!input_available()) {
        if (current_process)
            current_process->state = PROCESS_STATE_BLOCKED;
        r->eax = 0;
        return;
    }

    unsigned int i = 0;
    while (i < max && input_available()) {
        buf[i++] = input_getchar();
    }
    r->eax = i;
}

static void sys_open(struct registers *r) {
    char *path = (char *)r->ebx;
    if (!current_process) { r->eax = -1; return; }

    int fd = -1;
    for (int i = 3; i < MAX_FDS; i++) {
        if (!current_process->fds[i].used) { fd = i; break; }
    }
    if (fd == -1) { r->eax = -1; return; }

    fs_file_t f = fs_open(path);
    if (f.data == 0) { r->eax = -1; return; }

    current_process->fds[fd].data = f.data;
    current_process->fds[fd].size = f.size;
    current_process->fds[fd].offset = 0;
    current_process->fds[fd].used = 1;
    r->eax = fd;
}

static void sys_close(struct registers *r) {
    int fd = (int)r->ebx;
    if (!current_process || fd < 0 || fd >= MAX_FDS) { r->eax = -1; return; }
    current_process->fds[fd].used = 0;
    r->eax = 0;
}

static void sys_write(struct registers *r) {
    int fd = (int)r->ebx;
    char *buf = (char *)r->ecx;
    unsigned int count = r->edx;

    if (fd == 1 || fd == 2) {
        for (unsigned int i = 0; i < count; i++) {
            char tmp[2] = { buf[i], 0 };
            vga_print(tmp);
        }
        r->eax = count;
        return;
    }
    r->eax = -1;
}

static void sys_brk(struct registers *r) {
    unsigned int new_brk = r->ebx;
    if (!current_process) { r->eax = 1; return; }

    if (new_brk == 0) {
        r->eax = current_process->brk;
        return;
    }

    unsigned int old_brk = current_process->brk;

    unsigned int old_page = (old_brk + 0xFFF) & ~0xFFF;
    unsigned int new_page = (new_brk + 0xFFF) & ~0xFFF;

    for (unsigned int v = old_page; v < new_page; v += 4096) {
        unsigned int phys = paging_alloc_phys_frame();
        paging_map_page((unsigned int)current_process->page_dir, v, phys);
    }

    current_process->brk = new_brk;
    r->eax = new_brk;

}

static void sys_sleep(struct registers *r) {
    unsigned int ms = r->ebx;
    if (!current_process) return;

    unsigned int ticks_to_wait = ms / 10;
    unsigned int current = timer_ticks();
    unsigned int wake = current + ticks_to_wait;

    vga_print("ticks now: "); vga_print_num(current); vga_print("\n");
    vga_print("wake at: "); vga_print_num(wake); vga_print("\n");

    current_process->wake_tick = wake;
    current_process->state = PROCESS_STATE_SLEEPING;
    r->eax = 0;
}

void syscall_handler(struct registers *r) {
    switch (r->eax) {
        case SYSCALL_PRINT: sys_print(r); break;
        case SYSCALL_EXIT: sys_exit(r); break;
        case SYSCALL_READ: sys_read(r); break;
        case SYSCALL_OPEN: sys_open(r); break;
        case SYSCALL_CLOSE: sys_close(r); break;
        case SYSCALL_WRITE: sys_write(r); break;
        case SYSCALL_BRK: sys_brk(r); break;
        case SYSCALL_SLEEP: sys_sleep(r); break;
        default:
            vga_print("\nunknown syscall\n");
            break;
    }
}

void syscall_install() {
    idt_set_gate(128, (unsigned int)isr128, 0x08, 0xEE);
}
