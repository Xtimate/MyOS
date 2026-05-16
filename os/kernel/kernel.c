#include "gdt.h"
#include "idt.h"
#include "isr.h"
#include "irq.h"
#include "keyboard.h"

void kernel_main() {
    gdt_install();
    idt_install();
    isr_install();
    irq_install();
    keyboard_install();

    __asm__ volatile ("sti");

    char *vga = (char *)0xB8000;

    const char *msg = "Ready: ";
    int i = 0;
    while (msg[i] != 0) {
        vga[i * 2] = msg[i];
        vga[i * 2 + 1] = 0x07;
        i++;
    }

    while (1) {}
}
