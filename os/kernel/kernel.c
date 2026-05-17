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

void kernel_main() {
    gdt_install();
    idt_install();
    isr_install();
    irq_install();
    keyboard_install();
    timer_install(100);
    vga_init();
    kmalloc_init(0x500000, 0x600000);
    paging_init();

    __asm__ volatile ("sti");

    shell_init();

    while (1) {}
}
