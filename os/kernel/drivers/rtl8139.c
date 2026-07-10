#include "rtl8139.h"
#include "../include/pci.h"
#include "../include/framebuffer.h"
#include "../include/kmalloc.h"

#define PCI_COMMAND_REG 0x04
#define PCI_CMD_BUS_MASTER 0x4

#define RTL_REG_CONFIG1 0x52
#define RTL_REG_CMD 0x37
#define RTL_CMD_RESET 0x10

#define RTL_RX_BUFFER_SIZE (8192 + 16 + 1500)
#define RTL_REG_RBSTART 0x30
#define RTL_REG_RCR 0x44

#define RTL_RCR_AAP 0x01
#define RTL_RCR_APM 0x02
#define RTL_RCR_AM 0x04
#define RTL_RCR_AB 0x08
#define RTL_RCR_WRAP 0x80

#define RTL_CMD_RE 0x08
#define RTL_CMD_TE 0x04

#define RTL_REG_MAC0 0x00

#define RTL_REG_TSAD0 0x20
#define RTL_REG_TSD0 0x10

#define TX_BUFFER_SIZE 1536
#define TX_SLOT_COUNT 4

#define RTL_REG_CAPR 0x38

#define RTL_CMD_BUFE 0x01

static unsigned char *tx_buffers[TX_SLOT_COUNT];
static unsigned int tx_phys[TX_SLOT_COUNT];
static int tx_cur_slot = 0;

static unsigned char *rx_buffer = 0;
static unsigned int io_base = 0;
static unsigned char mac[6];

static unsigned int rx_offset = 0;

void rtl8139_get_mac(unsigned char *out) {
    for (int i = 0; i < 6; i++) {
        out[i] = mac[i];
    }
}

static void outb(unsigned short port, unsigned char val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static unsigned char inb(unsigned short port) {
    unsigned char val;
    __asm__ volatile ("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

static void outl(unsigned short port, unsigned int val) {
    __asm__ volatile ("outl %0, %1" : : "a"(val), "Nd"(port));
}

static void outw(unsigned short port, unsigned short val) {
    __asm__ volatile ("outw %0, %1" : : "a"(val), "Nd"(port));
}

static void outw_capr(int value) {
    if (value < 0) value += 8192;
    outw(io_base + RTL_REG_CAPR, (unsigned short)value);
}

static unsigned short htons(unsigned short x) {
    return (x << 8) | (x >> 8);
}

static unsigned int htonl(unsigned int x) {
    return  ((x & 0xFF) << 24) | ((x & 0xFF00) << 8) |
            ((x & 0xFF0000) >> 8) | ((x >> 24) & 0xFF);
}

void rtl8139_send(const unsigned char *data, unsigned int len) {
    if (len > TX_BUFFER_SIZE) len = TX_BUFFER_SIZE;

    int slot = tx_cur_slot;
    tx_cur_slot = (tx_cur_slot + 1) % TX_SLOT_COUNT;

    for (unsigned int i = 0; i < len; i++) {
        tx_buffers[slot][i] = data[i];
    }

    outl(io_base + RTL_REG_TSAD0 + slot * 4, tx_phys[slot]);
    outl(io_base + RTL_REG_TSD0 + slot * 4, len);
}

unsigned int rtl8139_receive(unsigned char *out) {
    if (inb(io_base + RTL_REG_CMD) & RTL_CMD_BUFE) {
        return 0;
    }

    unsigned char *packet = rx_buffer + rx_offset;

    unsigned short status = packet[0] | (packet[1] << 8);
    unsigned short length = packet[2] | (packet[3] << 8);

    (void)status;

    unsigned int data_len = length - 4;
    unsigned char *data = packet + 4;

    for (unsigned int i = 0; i < data_len; i++) {
        out[i] = data[i];
    }

    rx_offset = (rx_offset + length + 4 + 3) & ~3;
    rx_offset %= 8192;

    outw_capr(rx_offset - 16);

    return data_len;
}

int rtl8139_init() {
    pci_device_t *nic = pci_find_device(0x02, 0x00);
    if (!nic) return 0;

    io_base = pci_get_io_base(nic);
    if (io_base == 0) return 0;

    unsigned int cmd = pci_read32(nic->bus, nic->device, nic->function, PCI_COMMAND_REG);
    cmd |= PCI_CMD_BUS_MASTER;
    pci_write32(nic->bus, nic->device, nic->function, PCI_COMMAND_REG, cmd);

    outb(io_base + RTL_REG_CONFIG1, 0x00);
    outb(io_base + RTL_REG_CMD, RTL_CMD_RESET);

    int timeout = 1000000;
    while ((inb(io_base + RTL_REG_CMD) & RTL_CMD_RESET) && timeout--) {

    }

    if (timeout <= 0) {
        fb_terminal_print("RTL8139: reset timed out\n");
        return 0;
    }

    fb_terminal_print("RTL8139: reset ok\n");

    rx_buffer = (unsigned char *)kmalloc(RTL_RX_BUFFER_SIZE);
    if (!rx_buffer) {
        fb_terminal_print("RTL8139: failed to allocate RX buffer\n");
        return 0;
    }

    unsigned int rx_phys = (unsigned int)rx_buffer - 0xC0000000;

    outl(io_base + RTL_REG_RBSTART, rx_phys);

    fb_terminal_print("RTL8139: RX buffer set at phys ");
    pci_print_hex(rx_phys);
    fb_terminal_print("\n");

    outl(io_base + RTL_REG_RCR, RTL_RCR_AB | RTL_RCR_AM | RTL_RCR_APM | RTL_RCR_WRAP);
    outb(io_base + RTL_REG_CMD, RTL_CMD_RE | RTL_CMD_TE);

    fb_terminal_print("RTL8139: RX/TX enabled\n");

    for (int i = 0; i < 6; i++) {
        mac[i] = inb(io_base + RTL_REG_MAC0 + i);
    }

    fb_terminal_print("RTL8139: MAC ");
    for (int i = 0; i < 6; i++) {
        pci_print_hex(mac[i]);
        if (i < 5) fb_terminal_print(":");
    }
    fb_terminal_print("\n");

    for (int i = 0; i < TX_SLOT_COUNT; i++) {
        tx_buffers[i] = (unsigned char *)kmalloc(TX_BUFFER_SIZE);
        if (!tx_buffers[i]) {
            fb_terminal_print("RTL8139: failed to allocate TX buffer\n");
            return 0;
        }
        tx_phys[i] = (unsigned int)tx_buffers[i] - 0xC0000000;
    }

    fb_terminal_print("RTL8139: TX buffers allocated\n");

    return 1;
}
