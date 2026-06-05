#include "vga.h"
#include "include/framebuffer.h"
#define VGA_BUFFER 0xC00B8000

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
    fb_terminal_init();
}

void vga_set_color(unsigned char fg, unsigned char bg) {
    (void)fg; (void)bg;
}

void vga_putchar(char c) {
    fb_terminal_putchar(c);
}

void vga_print(const char *str) {
    fb_terminal_print(str);
}

void vga_print_num(unsigned int n) {
    fb_terminal_print_num(n);
}

void vga_print_hex(unsigned int n) {
    vga_print("0x");
    char buf[8];
    int i = 0;
    if (n == 0) {
        vga_print("00000000");
        return;
    }
    while (n > 0) {
        int nibble = n & 0xF;
        buf[i++] = nibble < 10 ? '0' + nibble : 'a' + nibble - 10;
        n >>= 4;
    }

    while (i < 8) buf[i++] = '0';
    for (int j = i - 1; j >= 0; j--) {
        vga_putchar(buf[j]);
    }
}
