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

#define USER_STACK_TOP 0x00300000

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
    vga_print("user_main addr: ");
    vga_print_hex((unsigned int)user_main);
    vga_print("\n");
    vga_print("user_stack top: ");
    vga_print_hex(USER_STACK_TOP);
    vga_print("\n");
    //jump_usermode(user_main, USER_STACK_TOP);

    while (1) {}
}
