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
static int shift_held = 0;

static const char scancode_map[] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0,
    0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0,
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' '
};

static const char scancode_map_shift[] = {
    0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0,
    0, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 0,
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
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
    if (scancode == 0x2A) { shift_held = 1; return; }
    if (scancode == 0xAA) { shift_held = 0; return; }

    if (scancode & 0x80) return;

    // Ctrl+C: kill all user processes
    if (ctrl_held && scancode == 0x2E) {
        for (int i = 0; i < MAX_PROCESSES; i++) {
            if (processes[i].type == PROCESS_TYPE_USER && processes[i].state != PROCESS_STATE_FREE) {
                process_exit(&processes[i]);
            }
        }
        input_init();
        foreground_pid = -1;
        vga_print("\n^C\n");
        return;
    }

    // Ctrl+Z: give focus back to shell
    if (ctrl_held && scancode == 0x2C) {
        foreground_pid = -1;
        vga_print("\n[shell]\n");
        return;
    }

    char c = 0;
    if (scancode == 0x1C) c = '\n';
    else if (scancode == 0x0E) c = '\b';
    else if (scancode < sizeof(scancode_map)) {
        c = shift_held ? scancode_map_shift[scancode] : scancode_map[scancode];
    }

    if (c == 0) return;

    // route to foreground
    if (foreground_pid != -1) {
        // unblock the foreground process if it's waiting on input
        for (int i = 0; i < MAX_PROCESSES; i++) {
            if (processes[i].pid == foreground_pid &&
                processes[i].state == PROCESS_STATE_BLOCKED) {
                processes[i].state = PROCESS_STATE_READY;
                break;
            }
        }
        input_putchar(c);
        return;
    }

    shell_process_char(c);
}

void keyboard_install() {
    irq_install_handler(1, keyboard_handler);
}
