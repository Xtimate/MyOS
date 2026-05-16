#include "shell.h"
#include "vga.h"

#define BUFFER_SIZE 256

static char buffer[BUFFER_SIZE];
static int buf_pos = 0;

// command handler type
typedef void (*command_func)();

struct command {
    const char *name;
    command_func func;
};

// forward declarations
static void cmd_hello();
static void cmd_help();
static void cmd_clear();

static struct command commands[] = {
    { "hello", cmd_hello },
    { "help",  cmd_help  },
    { "clear", cmd_clear },
    { 0, 0 }  // null terminator
};

// string compare since we have no stdlib
static int str_equal(const char *a, const char *b) {
    int i = 0;
    while (a[i] && b[i]) {
        if (a[i] != b[i]) return 0;
        i++;
    }
    return a[i] == b[i];
}

static void cmd_hello() {
    vga_print("\nHello from myOS!\n");
}

static void cmd_help() {
    vga_print("\nAvailable commands:\n");
    for (int i = 0; commands[i].name != 0; i++) {
        vga_print("  ");
        vga_print(commands[i].name);
        vga_print("\n");
    }
}

static void cmd_clear() {
    vga_clear();
}

static void shell_execute(const char *cmd) {
    if (buf_pos == 0) return;

    for (int i = 0; commands[i].name != 0; i++) {
        if (str_equal(cmd, commands[i].name)) {
            commands[i].func();
            return;
        }
    }

    vga_print("\nUnknown command: ");
    vga_print(cmd);
    vga_print("\n");
}

static void shell_prompt() {
    vga_print("\n> ");
}

void shell_init() {
    vga_print("Welcome to myOS\n");
    shell_prompt();
}

void shell_process_char(char c) {
    if (c == '\n') {
        buffer[buf_pos] = 0;
        shell_execute(buffer);
        buf_pos = 0;
        shell_prompt();
    } else if (c == '\b') {
        if (buf_pos > 0) {
            buf_pos--;
            vga_putchar('\b');
        }
    } else {
        if (buf_pos < BUFFER_SIZE - 1) {
            buffer[buf_pos++] = c;
            vga_putchar(c);
        }
    }
}
