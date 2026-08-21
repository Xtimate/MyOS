#include "include/cursor.h"
#include "include/mouse.h"
#include "include/framebuffer.h"

#define CURSOR_W 12
#define CURSOR_H 16

static const unsigned short cursor_bitmap[CURSOR_H] = {
    0b1000000000000000,
        0b1100000000000000,
        0b1110000000000000,
        0b1111000000000000,
        0b1111100000000000,
        0b1111110000000000,
        0b1111111000000000,
        0b1111111100000000,
        0b1111111110000000,
        0b1111111111000000,
        0b1111100000000000,
        0b1110110000000000,
        0b1100110000000000,
        0b1000011000000000,
        0b0000011000000000,
        0b0000001100000000,
};

static unsigned int backup[CURSOR_H][CURSOR_W];
static int last_x = -1;
static int last_y = -1;
static int drawn = 0;

void cursor_init() {
    last_x = -1;
    last_y = -1;
    drawn = 0;
}

static void cursor_erase() {
    if (!drawn) return;
    for (int row = 0; row < CURSOR_H; row++)
        for (int col = 0; col < CURSOR_W; col++)
            fb_put_pixel(last_x + col, last_y + row, backup[row][col]);
    drawn = 0;
}

static void cursor_draw(int x, int y) {
    for (int row = 0; row < CURSOR_H; row++)
        for (int col = 0; col < CURSOR_W; col++)
            backup[row][col] = fb_get_pixel(x + col, y + row);

    for (int row = 0; row < CURSOR_H; row ++) {
        unsigned short bits = cursor_bitmap[row];
        for (int col = 0; col < CURSOR_W; col++) {
            if (bits & (0x8000 >> col))
                fb_put_pixel(x + col, y + row, 0x00FFFFFF);
        }
    }

    last_x = x;
    last_y = y;
    drawn = 1;
}

void cursor_update() {
    int x = mouse_get_x();
    int y = mouse_get_y();
    if (drawn && x == last_x && y == last_y) return;
    cursor_erase();
    cursor_draw(x, y);
}

void cursor_notify_dirty() {
    drawn = 0;
}
