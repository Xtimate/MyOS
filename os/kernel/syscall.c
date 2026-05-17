#include "syscall.h"
#include "idt.h"
#include "vga.h"
#include "shell.h"

extern void isr128();

static void sys_print(struct registers *r) {
    char *str = (char *)r->ebx;
    vga_print(str);
}

static void sys_exit(struct registers *r) {
    vga_print("\n[Process exited]\n");
    shell_run();
}

void syscall_handler(struct registers *r) {
    switch (r->eax) {
        case SYSCALL_PRINT: sys_print(r); break;
        case SYSCALL_EXIT: sys_exit(r); break;
        default:
            vga_print("\nunknown syscall\n");
            break;
    }
}

void syscall_install() {
    idt_set_gate(128, (unsigned int)isr128, 0x08, 0xEE);
}
