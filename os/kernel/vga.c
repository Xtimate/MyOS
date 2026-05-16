#include "vga.h"
#define VGA_BUFFER 0xB8000

static int cursor_x = 0;
static int cursor_y = 0;
static unsigned char current_color = 0x07;

static void outb(unsigned short port, unsigned char val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static void update_cursor() {
    int pos = cursor_y * VGA_WIDTH + cursor_x;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (unsigned char)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char)((pos >> 8) & 0xFF));
}

static void scroll() {
    char *vga = (char *)VGA_BUFFER;
    if (cursor_y >= VGA_HEIGHT) {
        for (int i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH * 2; i++) {
            vga[i] = vga[i + VGA_WIDTH * 2];
        }

        for (int i = (VGA_HEIGHT - 1) * VGA_WIDTH * 2; i < VGA_HEIGHT * VGA_WIDTH * 2; i += 2) {
            vga[i] = ' ';
            vga[i + 1] = current_color;
        }
        cursor_y = VGA_HEIGHT - 1;
    }
}

void vga_init() {
    vga_clear();
}

void vga_clear() {
    char *vga = (char *)VGA_BUFFER;
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT * 2; i += 2) {
        vga[i] = ' ';
        vga[i + 1] = current_color;
    }
    cursor_x = 0;
    cursor_y = 0;
    update_cursor();
}

void vga_set_color(unsigned char fg, unsigned char bg) {
    current_color = (bg << 4) | (fg & 0x0F);
}

void vga_putchar(char c) {
    char *vga = (char *)VGA_BUFFER;

    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\r') {
        cursor_x = 0;
    } else if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
            int pos = (cursor_y * VGA_WIDTH + cursor_x) * 2;
            vga[pos] = ' ';
            vga[pos + 1] = current_color;
        }
    } else {
        int pos = (cursor_y * VGA_WIDTH + cursor_x) * 2;
        vga[pos] = c;
        vga[pos + 1] = current_color;
        cursor_x++;
        if (cursor_x > VGA_WIDTH) {
            cursor_x = 0;
            cursor_y++;
        }
    }

    scroll();
    update_cursor();
}

void vga_print(const char *str) {
    for (int i = 0; str[i] != 0; i++) {
        vga_putchar(str[i]);
    }
}
