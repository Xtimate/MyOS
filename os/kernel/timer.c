#include "include/timer.h"
#include "include/irq.h"
#include "include/process.h"

int kb_count = 0;

static unsigned int ticks = 0;

static void outb(unsigned short port, unsigned char val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static void timer_handler(struct registers *r) {
    ticks++;

    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].state == PROCESS_STATE_SLEEPING && ticks >= processes[i].wake_tick) {
            processes[i].state = PROCESS_STATE_READY;
        }
    }

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
