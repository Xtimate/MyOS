#include "gdt.h"
#include "vga.h"

struct gdt_entry gdt[6];
struct gdt_ptr gp;
struct tss_entry tss;

static unsigned char kernel_stack[8192];

void gdt_set_entry(int num, unsigned long base, unsigned long limit, unsigned char access, unsigned char gran) {
    gdt[num].base_low    = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high   = (base >> 24) & 0xFF;
    gdt[num].limit_low   = (limit & 0xFFFF);
    gdt[num].granularity = (limit >> 16) & 0x0F;
    gdt[num].granularity |= gran & 0xF0;
    gdt[num].access      = access;
}

static void tss_set(int num, unsigned short ss0, unsigned int esp0) {
    unsigned char *p = (unsigned char *)&tss;
    for (unsigned int i = 0; i < sizeof(struct tss_entry); i++) p[i] = 0;

    unsigned int base  = (unsigned int)&tss;
    unsigned int limit = sizeof(struct tss_entry) - 1;

    gdt_set_entry(num, base, limit, 0x89, 0x00);

    tss.ss0  = ss0;
    tss.esp0 = esp0;
    tss.cs   = 0x0B;
    tss.ss = tss.ds = tss.es = tss.fs = tss.gs = 0x13;
}

void gdt_install() {
    gp.limit = (sizeof(struct gdt_entry) * 6) - 1;
    gp.base  = (unsigned int)&gdt;

    gdt_set_entry(0, 0, 0, 0, 0);
    gdt_set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
    gdt_set_entry(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
    gdt_set_entry(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);
    gdt_set_entry(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);
    tss_set(5, 0x10, (unsigned int)(kernel_stack + 8192));

    gdt_flush((unsigned int)&gp);
    tss_flush();

}

void tss_set_kernel_stack(unsigned int esp0) {
    tss.esp0 = esp0;
}
