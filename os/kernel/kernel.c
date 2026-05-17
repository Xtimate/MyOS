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
#include "elf.h"

#define USER_STACK_TOP 0x00300000

extern char user_hello_start;
extern char user_hello_end;

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

    vga_print("Loading ELF...\n");
    unsigned int entry = elf_load(&user_hello_start);
    if (entry == 0) {
        vga_print("ELF load failed\n");
        while (1) {}
    }

    vga_print("Jumping to user mode at: ");
    vga_print_hex(entry);
    vga_print("\n");

    jump_usermode((void (*)())entry, USER_STACK_TOP);

    while (1) {}
}
