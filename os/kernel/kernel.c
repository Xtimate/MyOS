#include "gdt.h"
#include "idt.h"
#include "isr.h"
#include "irq.h"
#include "keyboard.h"
#include "vga.h"
#include "shell.h"
#include "kmalloc.h"
#include "timer.h"
#include "paging.h"
#include "usermode.h"
#include "syscall.h"

static unsigned char user_stack[4096];

static const char *user_msg = "hello from user mode!\n";

void user_main() {
    __asm__ volatile (
        "mov $0, %%eax\n"
        "mov %0, %%ebx\n"
        "int $0x80\n"
        "mov $1, %%eax\n"
        "int $0x80\n"
        :
        : "r"(user_msg)
        : "eax", "ebx"
    );
    while (1) {}
}

void kernel_main() {
    gdt_install();
    idt_install();
    isr_install();
    syscall_install();
    irq_install();
    keyboard_install();
    timer_install(100);
    vga_init();
    kmalloc_init(0xC0500000, 0xC0600000);
    paging_init();

    __asm__ volatile ("sti");

    shell_init();
    jump_usermode(user_main, (unsigned int)(user_stack + 4096));

    while (1) {}
}
