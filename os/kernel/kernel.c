#include "gdt.h"
#include "idt.h"
#include "include/process.h"
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
#include "process.h"

#define USER_STACK_TOP 0x00300000

extern char fs_archive_start;
extern char fs_archive_end;
extern void kernel_thread_start(unsigned int new_esp);

void shell_thread() {
    shell_init();
    while (1) { __asm__ volatile ("hlt"); }
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
    process_init();

    __asm__ volatile ("sti");

    fs_init(&fs_archive_start, &fs_archive_end - &fs_archive_start);

    process_t *shell = process_create_kernel(shell_thread);
    current_process = shell;
    shell->state = PROCESS_STATE_RUNNING;

    kernel_thread_start(shell->esp);
    while (1) {}
}
