#include "keyboard.h"
#include "irq.h"
#include "shell.h"
#include "timer.h"

#define KEYBOARD_DATA_PORT 0x60

extern int kb_count;

static const char scancode_map[] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0,
    0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0,
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' '
};

static unsigned char inb(unsigned short port) {
    unsigned char val;
    __asm__ volatile ("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

static void keyboard_handler(struct registers *r) {
    kb_count++;
    unsigned char scancode = inb(KEYBOARD_DATA_PORT);
    if (scancode & 0x80) return;

    if (scancode == 0x1C) {
        shell_process_char('\n');
        return;
    }

    if (scancode == 0x0E) {
        shell_process_char('\b');
        return;
    }

    if (scancode < sizeof(scancode_map)) {
        char c = scancode_map[scancode];
        if (c != 0) {
            shell_process_char(c);
        }
    }
}

void keyboard_install() {
    irq_install_handler(1, keyboard_handler);
}
