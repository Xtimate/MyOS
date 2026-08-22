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
#include "include/pci.h"
#include "include/framebuffer.h"
#include "net/arp.h"
#include "net/icmp.h"
#include "net/udp.h"
#include "net/dhcp.h"

#define BUFFER_SIZE 256
#define MAX_ARGS 16
#define HISTORY_SIZE 16
#define MAX_FILES 64
#define PROMPT_LEN 2

static char buffer[BUFFER_SIZE];
static int buf_pos = 0;
static char history[HISTORY_SIZE][BUFFER_SIZE];
static int history_count = 0;
static int history_pos = -1;
static int cursor_pos = 0;
static char *redirect_target = 0;
static char *redirect_input = 0;

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
static void cmd_pci(int argc, char **argv);
static void cmd_arp(int argc, char **argv);
static void cmd_ping(int argc, char **argv);
static void cmd_udptest(int argc, char **argv);
static void cmd_dhcp(int argc, char **argv);

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
    { "pci", cmd_pci },
    { "arp", cmd_arp },
    { "ping", cmd_ping },
    { "udptest", cmd_udptest },
    { "dhcp", cmd_dhcp },
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
    shell_output("\nHello from myOS!\n");
}

static void cmd_help(int argc, char **argv) {
    shell_output("\nAvailable commands:\n");
    for (int i = 0; commands[i].name != 0; i++) {
        shell_output("  ");
        shell_output(commands[i].name);
        shell_output("\n");
    }
}

static void cmd_clear(int argc, char **argv) {
    vga_clear();
}

static void cmd_echo(int argc, char **argv) {
    shell_output("\n");
    for (int i = 1; i < argc; i++) {
        shell_output(argv[i]);
        if (i < argc - 1) shell_output(" ");
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
    exec(argv[1], argc -1, argv + 1, redirect_target, redirect_input);
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
        shell_output("\nUsage: cat <filename>\n");
        return;
    }

    fs_file_t f = fs_open(argv[1]);
    if (f.data == 0) {
        shell_output("\ncat: file not found\n");
        return;
    }

    shell_output("\n");
    for (unsigned int i = 0; i < f.size; i++) {
        char tmp[2] = { f.data[i], 0 };
        shell_output(tmp);
    }
    vga_print("\n");
}

static void cmd_arp(int argc, char **argv) {
    fb_terminal_print("\n");
    unsigned char target_ip[4] = {192, 168, 100, 1};
    unsigned char mac[6];

    if (arp_resolve(target_ip, mac)) {
        fb_terminal_print("resolved\n");
    } else {
        fb_terminal_print("ARP resolve timed out\n");
    }
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

static void cmd_pci(int argc, char **argv) {
    fb_terminal_print("\n");
    pci_enumerate();
    pci_print_devices();
}

static void cmd_ping(int argc, char **argv) {
    fb_terminal_print("\n");
    unsigned char target_ip[4] = {8, 8, 8, 8}; // Default to Google's DNS server
    icmp_ping(target_ip);
}

static void cmd_udptest(int argc, char **argv) {
    fb_terminal_print("\n");
    unsigned char target[4] = {192, 168, 100, 1};
    const char *msg = "hello from my os";
    udp_send(target, 12345, 9999, (const unsigned char *)msg, 17);
    fb_terminal_print("UDP: sent\n");
}

static void cmd_dhcp(int argc, char **argv) {
    fb_terminal_print("\n");
    unsigned char new_ip[4];
    if (dhcp_request_ip(new_ip)) {
        fb_terminal_print("DHCP: got IP ");
        fb_terminal_print_num(new_ip[0]); fb_terminal_print(".");
        fb_terminal_print_num(new_ip[1]); fb_terminal_print(".");
        fb_terminal_print_num(new_ip[2]); fb_terminal_print(".");
        fb_terminal_print_num(new_ip[3]); fb_terminal_print("\n");
    } else {
        fb_terminal_print("DHCP: failed\n");
    }
}

void shell_output(const char *str) {
    if (redirect_target) {
        fs_write(redirect_target, str, kstrlen(str), 1);
    } else {
        vga_print(str);
    }
}

static void shell_execute(char *input) {
    if (buf_pos == 0) return;

    char *argv[MAX_ARGS];
    int argc = tokenize(input, argv, MAX_ARGS);

    if (argc == 0) return;

    char *output_file = 0;
    char *input_file = 0;

    for (int i = 0; i < argc; i++) {
        if (kstrcmp(argv[i], ">") == 0) {
            if (i + 1 < argc) {
                output_file = argv[i + 1];
                argc = i;
            }
            break;
        }
        if (kstrcmp(argv[i], "<") == 0) {
            if (i + 1 < argc) {
                input_file = argv[i + 1];
                argc = i;
            }
            break;
        }
    }

    if (argc == 0) return;

    if (output_file) {
        fs_create(output_file);
        redirect_target = output_file;
    }
    if (input_file) {
        redirect_input = input_file;
    }


    for (int i = 0; commands[i].name != 0; i++) {
        if (kstrcmp(argv[0], commands[i].name) == 0) {
            commands[i].func(argc, argv);
            redirect_target = 0;
            redirect_input = 0;
            return;
        }
    }

    redirect_target = 0;
    redirect_input = 0;
    vga_print("\nUnknown command: ");
    vga_print(argv[0]);
    vga_print("\n");
}

void shell_prompt() {
    vga_print("\n> ");
}

void shell_init() {
    vga_print("Welcome to myOS\n");
    shell_prompt();
}

static void redraw_tail(int from, int old_len) {
    fb_terminal_set_cursor_x(PROMPT_LEN + from);
    for (int i = from; i < buf_pos; i++) {
        vga_putchar(buffer[i]);
    }
    // clear any leftover characters from a longer previous line
    for (int i = buf_pos; i < old_len; i++) {
        vga_putchar(' ');
    }
    fb_terminal_set_cursor_x(PROMPT_LEN + cursor_pos);
}

void shell_process_char(char c) {
    if (c == 0x11) { // Up
        if (history_count == 0) return;
        if (history_pos < history_count - 1) history_pos++;
        int old_len = buf_pos;
        kstrcpy(buffer, history[history_count - 1 - history_pos]);
        buf_pos = kstrlen(buffer);
        cursor_pos = buf_pos;
        redraw_tail(0, old_len);
        return;
    }
    if (c == 0x12) { // Down
        int old_len = buf_pos;
        if (history_pos <= 0) {
            history_pos = -1;
            buf_pos = 0;
            buffer[0] = 0;
        } else {
            history_pos--;
            kstrcpy(buffer, history[history_count - 1 - history_pos]);
            buf_pos = kstrlen(buffer);
        }
        cursor_pos = buf_pos;
        redraw_tail(0, old_len);
        return;
    }
    if (c == 0x13) { // Left
        if (cursor_pos > 0) {
            cursor_pos--;
            fb_terminal_set_cursor_x(PROMPT_LEN + cursor_pos);
        }
        return;
    }
    if (c == 0x14) { // Right
        if (cursor_pos < buf_pos) {
            cursor_pos++;
            fb_terminal_set_cursor_x(PROMPT_LEN + cursor_pos);
        }
        return;
    }

    if (c == '\n') {
        buffer[buf_pos] = 0;
        if (buf_pos > 0) {
            if (history_count < HISTORY_SIZE) {
                kstrcpy(history[history_count++], buffer);
            } else {
                for (int i = 1; i < HISTORY_SIZE; i++) kstrcpy(history[i-1], history[i]);
                kstrcpy(history[HISTORY_SIZE - 1], buffer);
            }
        }
        history_pos = -1;
        shell_execute(buffer);
        buf_pos = 0;
        cursor_pos = 0;
        shell_prompt();
        return;
    }

    if (c == '\b') {
        if (cursor_pos > 0) {
            for (int i = cursor_pos - 1; i < buf_pos - 1; i++) {
                buffer[i] = buffer[i + 1];
            }
            buf_pos--;
            cursor_pos--;
            redraw_tail(cursor_pos, buf_pos + 1);
        }
        return;
    }

    if (c == '\t') {
        int word_start = cursor_pos;
        while (word_start > 0 && buffer[word_start - 1] != ' ') word_start--;

        int word_len = cursor_pos - word_start;
        if (word_len == 0) return;

        char names[MAX_FILES][100];
        int count = fs_list(names, MAX_FILES);

        char *match = 0;
        int match_count = 0;
        for (int i = 0; i < count; i++) {
            if (kstrncmp(names[i], &buffer[word_start], word_len) == 0) {
                match = names[i];
                match_count++;
            }
        }

        if (match_count == 1) {
            int old_len = buf_pos;
            int match_len = kstrlen(match);
            for (int i = 0; i < match_len; i++) {
                buffer[word_start + i] = match[i];
            }
            buf_pos = word_start + match_len;
            cursor_pos = buf_pos;
            redraw_tail(word_start, old_len);
        } else if (match_count > 1) {
            vga_print("\n");
            for (int i = 0; i < count; i++) {
                if (kstrncmp(names[i], &buffer[word_start], word_len) == 0) {
                    vga_print(names[i]);
                    vga_print("  ");
                }
            }
            vga_print("\n");
            shell_prompt();
            for (int i = 0; i < buf_pos; i++) vga_putchar(buffer[i]);
            fb_terminal_set_cursor_x(PROMPT_LEN + cursor_pos);
        }

        return;
    }

    // normal character: insert at cursor_pos
    if (buf_pos < BUFFER_SIZE - 1) {
        for (int i = buf_pos; i > cursor_pos; i--) {
            buffer[i] = buffer[i - 1];
        }
        buffer[cursor_pos] = c;
        buf_pos++;
        cursor_pos++;
        redraw_tail(cursor_pos - 1, buf_pos - 1);
    }
}

void shell_run() {
    buf_pos = 0;
    shell_prompt();
    __asm__ volatile ("sti");
    while (1) { __asm__ volatile ("hlt"); }
}
