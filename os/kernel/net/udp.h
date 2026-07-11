#ifndef UDP_H
#define UDP_H

#include "net.h"

typedef struct __attribute__ ((packed)) {
    unsigned short src_port;
    unsigned short dest_port;
    unsigned short length;
    unsigned short checksum;
} udp_header_t;

void udp_init();

int udp_send(const unsigned char *dest_ip, unsigned short src_port, unsigned short dest_port, const unsigned char *data, unsigned int len);

typedef void (*udp_handler_t)(const unsigned char *src_ip, unsigned short src_port, const unsigned char *data, unsigned int len);
void udp_register_handler(unsigned short port, udp_handler_t handler);
void udp_set_ip(const unsigned char *ip);
void udp_handle_packet(const unsigned char *src_ip, const unsigned char *data, unsigned int len);

#endif
