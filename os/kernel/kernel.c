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
#include "fs.h"

#define USER_STACK_TOP 0x00300000

extern char fs_archive_start;
extern char fs_archive_end;

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

    fs_init(&fs_archive_start, &fs_archive_end - &fs_archive_start);

    shell_init();

    vga_print("Loading hello from fs...\n");
        fs_file_t f = fs_open("hello");
        if (f.data == 0) {
            vga_print("File not found\n");
            while (1) {}
        }

        unsigned int entry = elf_load(f.data);
        if (entry == 0) {
            vga_print("ELF load failed\n");
            while (1) {}
        }

        vga_print("Jumping to: ");
        vga_print_hex(entry);
        vga_print("\n");

        jump_usermode((void (*)())entry, USER_STACK_TOP);

        while (1) {}
}
