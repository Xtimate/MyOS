#include "include/mouse.h"
#include "include/irq.h"

#define MOUSE_DATA_PORT 0x60
#define MOUSE_STATUS_PORT 0x64
#define MOUSE_CMD_PORT 0x64

static int mouse_x = 400;
static int mouse_y = 300;
static int screen_w = 1920;
static int screen_h = 1080;

static unsigned char packet[3];
static int packet_index = 0;
static int left_btn = 0;
static int right_btn = 0;

static unsigned char inb(unsigned short port) {
    unsigned char val;
    __asm__ volatile ("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

static void outb(unsigned short port, unsigned char val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static void mouse_wait_write() {
    int timeout = 100000;
    while (timeout--) {
        if (!(inb(MOUSE_STATUS_PORT) & 0x02)) return;
    }
}

static void mouse_wait_read() {
    int timeout = 100000;
    while (timeout--) {
        if (inb(MOUSE_STATUS_PORT) & 0x01) return;
    }
}

static void mouse_write(unsigned char val) {
    mouse_wait_write();
    outb(MOUSE_CMD_PORT, 0xD4);
    mouse_wait_write();
    outb(MOUSE_DATA_PORT, val);
}

static unsigned char mouse_read() {
    mouse_wait_read();
    return inb(MOUSE_DATA_PORT);
}

static void mouse_handler(struct registers *r) {
    unsigned char data = inb(MOUSE_DATA_PORT);
    packet[packet_index++] = data;

    if (packet_index == 3) {
        packet_index = 0;

        unsigned char status = packet[0];
        int dx = packet[1];
        int dy = packet[2];

        if (status & 0x10) dx |= 0xFFFFFF00;
        if (status & 0x20) dy |= 0xFFFFFF00;

        mouse_x += dx;
        mouse_y -= dy;

        if (mouse_x < 0) mouse_x = 0;
        if (mouse_y < 0) mouse_y = 0;
        if (mouse_x >= screen_w) mouse_x = screen_w - 1;
        if (mouse_y >= screen_h) mouse_y = screen_h - 1;

        left_btn = status & 0x01;
        right_btn = status & 0x02;
    }
}

void mouse_install() {
    mouse_wait_write();
    outb(MOUSE_CMD_PORT, 0xA8);

    mouse_wait_write();
    outb(MOUSE_CMD_PORT, 0x20);
    mouse_wait_read();
    unsigned char status = inb(MOUSE_DATA_PORT);
    status |= 0x02;
    status &= ~0x20;

    mouse_wait_write();
    outb(MOUSE_CMD_PORT, 0x60);
    mouse_wait_write();
    outb(MOUSE_DATA_PORT, status);

    mouse_write(0xF6);
    mouse_read();

    mouse_write(0xF4);
    mouse_read();

    irq_install_handler(12, mouse_handler);
    irq_clear_mask(12);
}

int mouse_get_x() { return mouse_x; }
int mouse_get_y() { return mouse_y; }
int mouse_left_button() { return left_btn; }
int mouse_right_button() { return right_btn; }
