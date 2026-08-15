#include "include/idt.h"

struct idt_entry idt[256];
struct idt_ptr idtp;

void idt_set_gate(unsigned char num, unsigned int base, unsigned short sel, unsigned char flags) {
    idt[num].base_low = (base & 0xFFFF);
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].selector = sel;
    idt[num].zero = 0;
    idt[num].flags = flags;
}

void idt_install() {
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base = (unsigned int)&idt;

    unsigned char *ptr = (unsigned char *)&idt;
    for (int i =0; i < sizeof(idt); i++) {
        ptr[i] = 0;
    }

    idt_load((unsigned int)&idtp);
}
