#ifndef ICMP_H
#define ICMP_H

#include "net.h"

typedef struct __attribute__ ((packed)) {
    unsigned char type;
    unsigned char code;
    unsigned short checksum;
    unsigned short id;
    unsigned short seq;
} icmp_header_t;

#define ICMP_ECHO_REQUEST 8
#define ICMP_ECHO_REPLY 0

void icmp_handle_packet(const unsigned char *src_ip, const unsigned char *data, unsigned int len);

int icmp_ping(const unsigned char *dest_ip);
#endif
