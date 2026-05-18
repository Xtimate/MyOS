#include "keyboard.h"
#include "include/input.h"
#include "include/process.h"
#include "irq.h"
#include "shell.h"
#include "process.h"
#include "input.h"
#include "vga.h"

#define KEYBOARD_DATA_PORT 0x60

static int ctrl_held = 0;

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
    unsigned char scancode = inb(KEYBOARD_DATA_PORT);

    if (scancode == 0x1D) { ctrl_held = 1; return; }
    if (scancode == 0x9D) { ctrl_held = 0; return; }

    if (scancode & 0x80) return;

    if (ctrl_held && scancode == 0x2E) {
        for (int i = 0; i < MAX_PROCESSES; i++) {
            if (processes[i].type == PROCESS_TYPE_USER && processes[i].state != PROCESS_STATE_FREE) {
                process_exit(&processes[i]);
            }
        }
        input_init();
        vga_print("\n^C\n");
        return;
    }

    char c = 0;
    if (scancode == 0x1C) c = '\n';
    else if (scancode == 0x0E) c = '\b';
    else if (scancode < sizeof(scancode_map)) c = scancode_map[scancode];

    if (c == 0) return;

    // check if any user process exists (running or blocked)
    int has_user_process = 0;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].state != PROCESS_STATE_FREE &&
            processes[i].type == PROCESS_TYPE_USER) {
            if (processes[i].state == PROCESS_STATE_BLOCKED)
                processes[i].state = PROCESS_STATE_READY;
            has_user_process = 1;
        }
    }

    if (has_user_process) {
        input_putchar(c);
        return;
    }

    shell_process_char(c);
}

void keyboard_install() {
    irq_install_handler(1, keyboard_handler);
}
