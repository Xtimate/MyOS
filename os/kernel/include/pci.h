#ifndef PCI_H
#define PCI_H

typedef struct {
    unsigned short vendor;
    unsigned short device;
    unsigned char bus;
    unsigned char slot;
    unsigned char func;
    unsigned int io_base;
} pci_device_t;

unsigned int pci_read(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset);
int pci_find_device(unsigned short vendor, unsigned short device, pci_device_t *out);

#endif
