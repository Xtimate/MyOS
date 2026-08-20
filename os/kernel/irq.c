#include "include/irq.h"
#include "include/idt.h"

// PIC ports
#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

// function to write to a port
static void outb(unsigned short port, unsigned char val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

static void *irq_routines[16] = { 0 };

void irq_install_handler(int irq, void (*handler)(struct registers *r)) {
    irq_routines[irq] = handler;
}

void irq_uninstall_handler(int irq) {
    irq_routines[irq] = 0;
}

void irq_install() {
    // remap PIC to 32-47
    outb(PIC1_COMMAND, 0x11);
    outb(PIC2_COMMAND, 0x11);
    outb(PIC1_DATA,    0x20);  // PIC1 starts at 32
    outb(PIC2_DATA,    0x28);  // PIC2 starts at 40
    outb(PIC1_DATA,    0x04);
    outb(PIC2_DATA,    0x02);
    outb(PIC1_DATA,    0x01);
    outb(PIC2_DATA,    0x01);
    outb(PIC1_DATA,    0x00);
    outb(PIC2_DATA,    0x00);

    idt_set_gate(32, (unsigned int)irq0,  0x08, 0x8E);
    idt_set_gate(33, (unsigned int)irq1,  0x08, 0x8E);
    idt_set_gate(34, (unsigned int)irq2,  0x08, 0x8E);
    idt_set_gate(35, (unsigned int)irq3,  0x08, 0x8E);
    idt_set_gate(36, (unsigned int)irq4,  0x08, 0x8E);
    idt_set_gate(37, (unsigned int)irq5,  0x08, 0x8E);
    idt_set_gate(38, (unsigned int)irq6,  0x08, 0x8E);
    idt_set_gate(39, (unsigned int)irq7,  0x08, 0x8E);
    idt_set_gate(40, (unsigned int)irq8,  0x08, 0x8E);
    idt_set_gate(41, (unsigned int)irq9,  0x08, 0x8E);
    idt_set_gate(42, (unsigned int)irq10, 0x08, 0x8E);
    idt_set_gate(43, (unsigned int)irq11, 0x08, 0x8E);
    idt_set_gate(44, (unsigned int)irq12, 0x08, 0x8E);
    idt_set_gate(45, (unsigned int)irq13, 0x08, 0x8E);
    idt_set_gate(46, (unsigned int)irq14, 0x08, 0x8E);
    idt_set_gate(47, (unsigned int)irq15, 0x08, 0x8E);

    irq_clear_mask(0);
    irq_clear_mask(1);
    irq_clear_mask(2);
}

void irq_handler(struct registers *r) {
    void (*handler)(struct registers *r);
    handler = irq_routines[r->int_no - 32];
    if (handler) {
        handler(r);
    }

    if (r->int_no >= 40) {
        outb(PIC2_COMMAND, 0x20);
    }
    outb(PIC1_COMMAND, 0x20);
}

void irq_clear_mask(unsigned char irq) {
    unsigned short port;
    unsigned char value;

    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }

    unsigned char mask;
    __asm__ volatile ("inb %1, %0" : "=a"(mask) : "Nd"(port));
    value = mask & ~(1 << irq);
    outb(port, value);
}
