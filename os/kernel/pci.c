#include "include/pci.h"

static unsigned int inl(unsigned short port) {
    unsigned int val;
    __asm__ volatile ("inl %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

static void outl(unsigned short port, unsigned int val) {
    __asm__ volatile ("outl %0, %1" : : "a"(val), "Nd"(port));
}

unsigned int pci_read(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset) {
    unsigned int address = (1u << 31)
        | ((unsigned int)bus << 16)
        | ((unsigned int)slot << 11)
        | ((unsigned int)func << 8)
        | (offset & 0xFC);
    outl(0xCF8, address);
    return inl(0xCFC);
}

int pci_find_device(unsigned short vendor, unsigned short device, pci_device_t *out) {
    for (unsigned char bus = 0; bus < 256; bus++) {
        for (unsigned char slot = 0; slot < 32; slot++) {
            unsigned int val = pci_read(bus, slot, 0, 0);
            if ((val & 0xFFFF) == vendor && (val >> 16) == device) {
                out->vendor = vendor;
                out->device = device;
                out->bus = bus;
                out->slot = slot;
                out->func = 0;
                unsigned int bar0 = pci_read(bus, slot, 0, 0x10);
                out->io_base = bar0 & ~0x3;
                return 1;
            }
        }
    }
    return 0;
}
