#ifndef ARP_H
#define ARP_H

#include "net.h"

typedef struct __attribute__((packed)) {
    unsigned short htype;
    unsigned short ptype;
    unsigned char hlen;
    unsigned char plen;
    unsigned short oper;
    unsigned char send_mac[6];
    unsigned char sender_ip[4];
    unsigned char target_mac[6];
    unsigned char target_ip[4];
} arp_packet_t;

#define ARP_REQUEST 1
#define ARP_REPLY 2

void arp_init(const unsigned char *our_mac, const unsigned char *our_ip);

void arp_send_request(const unsigned char *target_ip);

void arp_handle_packet(const unsigned char *data, unsigned int len);

#endif
