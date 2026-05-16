#include "gdt.h"
#include "idt.h"
#include "isr.h"
#include "irq.h"
#include "keyboard.h"
#include "vga.h"
#include "shell.h"

void kernel_main() {
    gdt_install();
    idt_install();
    isr_install();
    irq_install();
    keyboard_install();
    vga_init();

    __asm__ volatile ("sti");

    shell_init();

    while (1) {}
}
