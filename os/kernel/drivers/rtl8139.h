#ifndef RTL8139_H
#define RTL8139_H

#include "../include/pci.h"

unsigned int rtl8139_receive(unsigned char *out);
void rtl8139_send(const unsigned char *data, unsigned int len);
void rtl8139_get_mac(unsigned char *out);
int rtl8139_init();

#endif
