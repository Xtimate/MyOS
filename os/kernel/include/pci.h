#ifndef PCI_H
#define PCI_H

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

#define PCI_VENDOR_ID 0x00
#define PCI_DEVICE_ID 0x02
#define PCI_COMMAND 0x04
#define PCI_STATUS 0x06
#define PCI_REVISION_ID 0x08
#define PCI_PROG_IF 0x09
#define PCI_SUBCLASS 0x0A
#define PCI_CLASS 0x0B
#define PCI_HEADER_TYPE 0x0E
#define PCI_BAR0 0x10
#define PCI_BAR1 0x14
#define PCI_BAR3 0x1C
#define PCI_BAR4 0x20
#define PCI_BAR5 0x24
#define PCI_INTERRUPT_LINE 0x3C

#define PCI_HEADER_TYPE_DEVICE 0x00
#define PCI_HEADER_TYPE_BRIDGE 0x01
#define PCI_MULTIFUNCTION 0x80

typedef struct {
    unsigned char bus;
    unsigned char device;
    unsigned char function;
    unsigned short vendor_id;
    unsigned short device_id;
    unsigned char class_code;
    unsigned char subclass;
    unsigned char prog_if;
    unsigned char header_type;
    unsigned char interrupt_line;
} pci_device_t;

#define PCI_MAX_DEVICES 64

unsigned int pci_read32(unsigned char bus, unsigned char dev, unsigned char func, unsigned char offset);

unsigned short pci_read16(unsigned char bus, unsigned char dev, unsigned char func, unsigned char offset);

unsigned char pci_read8(unsigned char bus, unsigned char dev, unsigned char func, unsigned char offset);

void pci_write32(unsigned char bus, unsigned char dev, unsigned char func, unsigned char offset, unsigned int value);

void pci_enumerate();

void pci_print_devices();

void pci_print_hex(unsigned int n);

pci_device_t *pci_find_device(unsigned char class_code, unsigned char subclass);

unsigned int pci_get_io_base(pci_device_t *dev);

extern int pci_device_count;
extern pci_device_t pci_devices[PCI_MAX_DEVICES];
#endif
