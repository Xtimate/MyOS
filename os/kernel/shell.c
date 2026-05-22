#include "include/shell.h"
#include "include/process.h"
#include "include/vga.h"
#include "include/vga.h"
#include "include/kmalloc.h"
#include "include/timer.h"
#include "include/kstring.h"
#include "include/exec.h"
#include "include/process.h"
#include "include/fs.h"

#define BUFFER_SIZE 256
#define MAX_ARGS 16

static char buffer[BUFFER_SIZE];
static int buf_pos = 0;

// command handler type
typedef void (*command_func)(int argc, char **argv);

struct command {
    const char *name;
    command_func func;
};

// forward declarations
static void cmd_hello(int argc, char **argv);
static void cmd_help(int argc, char **argv);
static void cmd_clear(int argc, char **argv);
static void cmd_mem(int argc, char **argv);
static void cmd_uptime(int argc, char **argv);
static void cmd_echo(int argc, char **argv);
static void cmd_color(int argc, char **argv);
static void cmd_meminfo(int argc, char **argv);
static void cmd_exec(int argc, char **argv);
static void cmd_ps(int argc, char **argv);
static void cmd_kill(int argc, char **argv);
static void cmd_cat(int argc, char **argv);
static void cmd_fg(int argc, char **argv);

static struct command commands[] = {
    { "hello", cmd_hello },
    { "help",  cmd_help  },
    { "clear", cmd_clear },
    { "mem", cmd_mem },
    { "uptime", cmd_uptime },
    { "echo", cmd_echo },
    { "color", cmd_color },
    { "meminfo", cmd_meminfo },
    { "exec", cmd_exec },
    { "ps", cmd_ps },
    { "kill", cmd_kill },
    { "cat", cmd_cat },
    { "fg", cmd_fg },
    { 0, 0 }  // null terminator
};

static void cmd_meminfo(int argc, char **argv) {
    kmalloc_info();
}

static int tokenize(char *input, char **argv, int max_args) {
    int argc = 0;
    char *p = input;

    while (*p && argc < max_args) {
        while (*p == ' ') p++;
        if (*p == '\0') break;

        argv[argc++] = p;

        while (*p && *p != ' ') p++;

        if (*p == ' ') {
            *p = '\0';
            p++;
        }
    }

    return argc;
}

static void cmd_mem(int argc, char **argv) {
    vga_print("\nUsed: ");
    vga_print_num(kmalloc_used());
    vga_print(" bytes\nFree: ");
    vga_print_num(kmalloc_free());
    vga_print(" bytes\n");
}

static void cmd_uptime(int argc, char **argv) {
    vga_print("\nUptime: ");
    vga_print_num(timer_ticks() / 100);
    vga_print(" seconds \n");
}

static void cmd_hello(int argc, char **argv) {
    vga_print("\nHello from myOS!\n");
}

static void cmd_help(int argc, char **argv) {
    vga_print("\nAvailable commands:\n");
    for (int i = 0; commands[i].name != 0; i++) {
        vga_print("  ");
        vga_print(commands[i].name);
        vga_print("\n");
    }
}

static void cmd_clear(int argc, char **argv) {
    vga_clear();
}

static void cmd_echo(int argc, char **argv) {
    vga_print("\n");
    for (int i = 1; i < argc; i++) {
        vga_print(argv[i]);
        if (i < argc - 1) vga_print(" ");
    }
    vga_print("\n");
}

static void cmd_color(int argc, char **argv) {
    if (argc < 2) {
        vga_print("\nUsage: color <0-15>\n");
        return;
    }

    int color = 0;
    char *s = argv[1];
    while (*s >= '0' && *s <= '9') {
        color = color * 10 + (*s - '0');
        s++;
    }

    if (color > 15) {
        vga_print("\nColor must be 0-15\n");
        return;
    }
    vga_set_color((unsigned char)color, 0);
    vga_print("\nColor set.\n");
}

static void cmd_exec(int argc, char **argv) {
    if (argc < 2) {
        vga_print("\nUsage: exec <filename>\n");
        return;
    }
    vga_print("\n");
    exec(argv[1], argc -1, argv + 1);
}

static void cmd_ps(int argc, char **argv) {
    vga_print("\nPID  TYPE    STATE\n");
    vga_print("---  ------  -----\n");
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].state == PROCESS_STATE_FREE) continue;

        vga_print_num(processes[i].pid);
        vga_print("    ");

        if (processes[i].type == PROCESS_TYPE_KERNEL)
            vga_print("kernel  ");
        else
            vga_print("user  ");

        switch (processes[i].state) {
            case PROCESS_STATE_READY: vga_print("ready");  break;
            case PROCESS_STATE_RUNNING: vga_print("running"); break;
            default: vga_print("unknown"); break;
        }
        vga_print("\n");
    }
}

static void cmd_kill(int argc, char **argv) {
    if (argc < 2) {
        vga_print("Usage: kill <pid>\n");
        return;
    }

    int pid = 0;
    char *s = argv[1];
    while (*s >= '0' && *s <= '9') {
        pid = pid * 10 + (*s - '0');
        s++;
    }

    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].pid ==(unsigned int)pid) {
            if (processes[i].type == PROCESS_TYPE_KERNEL) {
                vga_print("\nkill: cannot kill kernel process\n");
                return;
            }
            process_exit(&processes[i]);
            vga_print("killed process ");
            vga_print_num(pid);
            vga_print("\n");
            return;
        }
    }

    vga_print("kill: no such process\n");
}

static void cmd_cat(int argc, char **argv) {
    if (argc < 2) {
        vga_print("\nUsage: cat <filename>\n");
        return;
    }

    fs_file_t f = fs_open(argv[1]);
    if (f.data == 0) {
        vga_print("\ncat: file not found\n");
        return;
    }

    vga_print("\n");
    for (unsigned int i = 0; i < f.size; i++) {
        char tmp[2] = { f.data[i], 0 };
        vga_print(tmp);
    }
    vga_print("\n");
}

static void cmd_fg(int argc, char **argv) {
    if (argc < 2) {
        vga_print("\nUsage: fg <pid>\n");
        return;
    }

    int pid = 0;
    char *s = argv[1];
    while (*s >= '0' && *s <= '9') {
        pid = pid * 10 + (*s - '0');
        s++;
    }

    for (int i = 0; i < MAX_PROCESSES; i++) {
        if ((int)processes[i].pid == pid && processes[i].state != PROCESS_STATE_FREE) {
            if (processes[i].type != PROCESS_TYPE_KERNEL) {
                foreground_pid = pid;
                vga_print("\n");
                return;
            }
            else {
                vga_print("\nfg: can't switch to kernel\n");
            }
        }
    }

    vga_print("\nfg: no such process\n");
}

static void shell_execute(char *input) {
    if (buf_pos == 0) return;

    char *argv[MAX_ARGS];
    int argc = tokenize(input, argv, MAX_ARGS);

    if (argc == 0) return;

    for (int i = 0; commands[i].name != 0; i++) {
        if (kstrcmp(argv[0], commands[i].name) == 0) {
            commands[i].func(argc, argv);
            return;
        }
    }

    vga_print("\nUnknown command: ");
    vga_print(argv[0]);
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

void shell_run() {
    buf_pos = 0;
    shell_prompt();
    __asm__ volatile ("sti");
    while (1) { __asm__ volatile ("hlt"); }
}
