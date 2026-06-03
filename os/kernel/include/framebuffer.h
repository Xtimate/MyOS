#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

void fb_init(unsigned char *addr, unsigned int width, unsigned int height, unsigned int pitch, unsigned int bpp);
void fb_put_pixel(unsigned int x, unsigned int y, unsigned int color);
void fb_fill_rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int color);
void fb_draw_char(unsigned int x, unsigned int y, char c, unsigned int fg, unsigned int bg);
void fb_draw_string(unsigned int x, unsigned int y, const char *str, unsigned int fg, unsigned int bg);
void fb_clear(unsigned int color);
void fb_terminal_init(void);
void fb_terminal_putchar(char c);
void fb_terminal_print(const char *str);
void fb_terminal_print_num(unsigned int n);

#endif
