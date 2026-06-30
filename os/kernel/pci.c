#include "include/pci.h"
#include "include/framebuffer.h"

static void outl(unsigned short port, unsigned int val) {
    __asm__ volatile ("outl %0, %1" : : "a"(val), "Nd"(port));
}

static unsigned int inl(unsigned short port) {
    unsigned int val;
    __asm__ volatile ("inl %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

void pci_print_hex(unsigned int n) {
    char buf[9];
    buf[8] = 0;
    for (int i = 7; i >= 0; i--) {
        int nibble = n & 0xF;
        buf[i] = nibble < 10 ? '0' + nibble : 'a' + nibble - 10;
        n >>= 4;
    }
    // skip leading zeros
    char *p = buf;
    while (*p == '0' && *(p+1)) p++;
    fb_terminal_print(p);
}

unsigned int pci_read32(unsigned char bus, unsigned char dev, unsigned char func, unsigned char offset) {
    unsigned int address =
        (1U << 31) |
        ((unsigned int)bus << 16) |
        ((unsigned int)dev << 11) |
        ((unsigned int)func << 8) |
        (offset & 0xFC);

    outl(PCI_CONFIG_ADDRESS, address);
    return inl(PCI_CONFIG_DATA);

}

unsigned short pci_read16(unsigned char bus, unsigned char dev, unsigned char func, unsigned char offset) {
    unsigned int val = pci_read32(bus, dev, func, offset & ~3);

    return (unsigned char)((val >> ((offset & 3) * 8)) & 0xFF);
}

unsigned char pci_read8(unsigned char bus, unsigned char dev,
                        unsigned char func, unsigned char offset) {
    unsigned int val = pci_read32(bus, dev, func, offset & ~3);
    // offset & 3 tells us which byte within the dword
    return (unsigned char)((val >> ((offset & 3) * 8)) & 0xFF);
}

void pci_write32(unsigned char bus, unsigned char dev, unsigned char func, unsigned char offset, unsigned int value) {
    unsigned int address =
        (1U << 31) |
        ((unsigned int)bus << 16) |
        ((unsigned int)dev << 11) |
        ((unsigned int)func << 8) |
        (offset & 0xFC);

    outl(PCI_CONFIG_ADDRESS, address);
    outl(PCI_CONFIG_DATA, value);
}

pci_device_t pci_devices[PCI_MAX_DEVICES];
int pci_device_count = 0;

static const char *pci_class_name(unsigned char class_code, unsigned char subclass) {
    (void)subclass;
    static const char *names[] = {
        "Unclassified",        // 0x00
        "Mass storage",        // 0x01
        "Network controller",  // 0x02
        "Display controller",  // 0x03
        "Multimedia device",   // 0x04
        "Memory controller",   // 0x05
        "Bridge device",       // 0x06
        "Communication ctrl",  // 0x07
        "System peripheral",   // 0x08
        "Input device",        // 0x09
        "Docking station",     // 0x0A
        "Processor",           // 0x0B
        "Serial bus ctrl",     // 0x0C
        "Wireless controller", // 0x0D
    };
    if (class_code < 0x0E)
        return names[class_code];
    return "Unknown device";
}

static void pci_check_function(unsigned char bus, unsigned char dev, unsigned char func) {
    unsigned short vendor = pci_read16(bus, dev, func, PCI_VENDOR_ID);

    if (vendor == 0xFFFF) return;

    if (pci_device_count >= PCI_MAX_DEVICES) return;

    pci_device_t *d = &pci_devices[pci_device_count++];
    d->bus =            bus;
    d->device =         dev;
    d->function =       func;
    d->vendor_id =      vendor;
    d->device_id =      pci_read16(bus, dev, func, PCI_DEVICE_ID);
    d->class_code =     pci_read8(bus, dev, func, PCI_CLASS);
    d->subclass =       pci_read8(bus, dev, func, PCI_SUBCLASS);
    d->prog_if =        pci_read8(bus, dev, func, PCI_PROG_IF);
    d->header_type =    pci_read8(bus, dev, func, PCI_HEADER_TYPE);
    d->interrupt_line = pci_read8(bus, dev, func, PCI_INTERRUPT_LINE);
}

static void pci_check_device(unsigned char bus, unsigned char dev) {
    unsigned short vendor = pci_read16(bus, dev, 0, PCI_VENDOR_ID);
    if (vendor == 0xFFFF) return;

    pci_check_function(bus, dev, 0);

    unsigned char header_type = pci_read8(bus, dev, 0, PCI_HEADER_TYPE);
    if (header_type & PCI_MULTIFUNCTION) {
        for (unsigned char func = 1; func < 8; func++) {
            if (pci_read16(bus, dev, func, PCI_VENDOR_ID) != 0xFFFF)
                pci_check_function(bus,dev, func);
        }
    }
}

void pci_enumerate() {
    pci_device_count = 0;

    for (unsigned int bus = 0; bus < 1; bus++) {
        for (unsigned char dev = 0; dev < 32; dev++) {
            pci_check_device((unsigned char)bus, dev);
        }
    }
}

void pci_print_devices() {
    if (pci_device_count == 0) {
        fb_terminal_print("No PCI devices found.\n");
        return;
    }

    for (int i; i < pci_device_count; i++) {
        pci_device_t *d = &pci_devices[i];

        fb_terminal_print_num(d->bus);
        fb_terminal_print(":");
        fb_terminal_print_num(d->device);
        fb_terminal_print(".");
        fb_terminal_print_num(d->function);
        fb_terminal_print("  ");

        pci_print_hex(d->vendor_id);
        fb_terminal_print(":");
        pci_print_hex(d->device_id);
        fb_terminal_print("  ");

        fb_terminal_print(pci_class_name(d->class_code, d->subclass));

        if (d->interrupt_line != 0xFF && d->interrupt_line != 0) {
            fb_terminal_print("  IRQ");
            fb_terminal_print_num(d->interrupt_line);
        }

        fb_terminal_print("\n");
    }
    fb_terminal_print_num(pci_device_count);
    fb_terminal_print(" device(s) found.\n");
}

pci_device_t *pci_find_device(unsigned char class_code, unsigned char subclass) {
    for (int i = 0; i < pci_device_count; i++) {
        if (pci_devices[i].class_code == class_code &&
            pci_devices[i].subclass   == subclass) {
                return &pci_devices[i];
            }
    }
    return 0;
}
