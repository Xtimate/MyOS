#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

void fb_init(unsigned char *addr, unsigned int width, unsigned int height, unsigned int pitch, unsigned int bpp);
void fb_put_pixel(unsigned int x, unsigned int y, unsigned int color);
void fb_fill_rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int color);
void fb_clear(unsigned int color);

#endif
