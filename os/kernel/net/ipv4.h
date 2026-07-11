#ifndef IPV4_H
#define IPV4_H

#include "net.h"

typedef struct __attribute__((packed)) {
    unsigned char ver_ihl;
    unsigned char tos;
    unsigned short total_len;
    unsigned short id;
    unsigned short flags_frag;
    unsigned char ttl;
    unsigned char protocol;
    unsigned short checksum;
    unsigned char src_ip[4];
    unsigned char dest_ip[4];
} ipv4_header_t;

#define IP_PROTO_ICMP 1
#define IP_PROTO_TCP 6
#define IP_PROTO_UDP 17

void ipv4_init(const unsigned char *our_ip);

int ipv4_send(const unsigned char *dest_ip, unsigned char protocol, const unsigned char *payload, unsigned int payload_len);

void ipv4_handle_packet(const unsigned char *data, unsigned int len);

#endif
