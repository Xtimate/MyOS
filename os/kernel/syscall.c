#include "syscall.h"
#include "idt.h"
#include "include/process.h"
#include "vga.h"
#include "shell.h"
#include "process.h"
#include "include/input.h"

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

static void sys_read(struct registers *r) {
    char *buf = (char *)r->ebx;
    unsigned int max = r->ecx;

    if (!input_available()) {
        if (current_process)
            current_process->state = PROCESS_STATE_BLOCKED;
        r->eax = 0;
        return;
    }

    vga_print("sys_read: got data\n");

    unsigned int i = 0;
    while (i < max && input_available()) {
        buf[i++] = input_getchar();
    }
    r->eax = i;
}

void syscall_handler(struct registers *r) {
    switch (r->eax) {
        case SYSCALL_PRINT: sys_print(r); break;
        case SYSCALL_EXIT: sys_exit(r); break;
        case SYSCALL_READ: sys_read(r); break;
        default:
            vga_print("\nunknown syscall\n");
            break;
    }
}

void syscall_install() {
    idt_set_gate(128, (unsigned int)isr128, 0x08, 0xEE);
}
