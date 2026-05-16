#ifndef VGA_H
#define VGA_H

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

void vga_init();
void vga_clear();
void vga_putchar(char c);
void vga_print(const char *str);
void vga_print_num(unsigned int n);
void vga_set_color(unsigned char fg, unsigned char bg);

#endif
