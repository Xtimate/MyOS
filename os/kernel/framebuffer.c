#include "include/framebuffer.h"

static unsigned char *fb_addr;
static unsigned int fb_width;
static unsigned int fb_height;
static unsigned int fb_pitch;
static unsigned int fb_bpp;

void fb_init(unsigned char *addr, unsigned int width, unsigned int height, unsigned int pitch, unsigned int bpp) {
    fb_addr = addr;
    fb_width = width;
    fb_height = height;
    fb_pitch = pitch;
    fb_bpp = bpp;
}

void fb_put_pixel(unsigned int x, unsigned int y, unsigned int color) {
    if (x >= fb_width || y >= fb_height) return;
    unsigned int *pixel = (unsigned int *)(fb_addr + y * fb_pitch + x * (fb_bpp / 8));
    *pixel = color;
}

void fb_fill_rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int color) {
    for (unsigned int row = y; row < y + h; row++) {
        for (unsigned int col = x; col < x + w; col++) {
            fb_put_pixel(col, row, color);
        }
    }
}

void fb_clear(unsigned int color) {
    fb_fill_rect(0, 0, fb_width, fb_height, color);
}
