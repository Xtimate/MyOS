#include "timer.h"
#include "irq.h"
#include "vga.h"
#include "process.h"

static unsigned int ticks = 0;

static void outb(unsigned short port, unsigned char val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static void timer_handler(struct registers *r) {
    ticks++;
    schedule(r);
}

void timer_install(unsigned int hz) {
    irq_install_handler(0, timer_handler);

    unsigned int divisor = 1193180 / hz;

    unsigned char low = (unsigned char)(divisor & 0xFF);
    unsigned char high = (unsigned char)((divisor >> 8) & 0xFF);

    __asm__ volatile ("outb %0, %1" : : "a"((unsigned char)0x36), "Nd"((unsigned short)0x43));
    __asm__ volatile ("outb %0, %1" : : "a"(low),  "Nd"((unsigned short)0x40));
    __asm__ volatile ("outb %0, %1" : : "a"(high), "Nd"((unsigned short)0x40));
}

unsigned int timer_ticks() {
    return ticks;
}
