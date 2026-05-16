#include "keyboard.h"
#include "irq.h"
#include "vga.h"

#define KEYBOARD_DATA_PORT 0x60

static void outb(unsigned short port, unsigned char val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static void update_cursor(int pos) {
    outb(0x3D4, 0x0F);
    outb(0x3D5, (unsigned char)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char)((pos >> 8) & 0xFF));
}

static const char scancode_map[] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0,
    0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0,
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' '
};

static int cursor = 6;

static unsigned char inb(unsigned short port) {
    unsigned char val;
    __asm__ volatile ("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

static void keyboard_handler(struct registers *r) {
    unsigned char scancode = inb(KEYBOARD_DATA_PORT);
    if (scancode & 0x80) return;

    if (scancode == 0x1C) {
        vga_putchar('\n');
        return;
    }

    if (scancode == 0x0E) {
        vga_putchar('\b');
        return;
    }
    if (scancode < sizeof(scancode_map)) {
        char c = scancode_map[scancode];
        if (c != 0) {
            vga_putchar(c);
        }
    }
}

void keyboard_install() {
    irq_install_handler(1, keyboard_handler);
}
