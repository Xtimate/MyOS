#ifndef DHCP_H
#define DHCP_H

#include "net.h"

typedef struct __attribute__ ((packed)) {
    unsigned char op;
    unsigned char htype;
    unsigned char hlen;
    unsigned char hops;
    unsigned int xid;
    unsigned short secs;
    unsigned short flags;
    unsigned char ciaddr[4];
    unsigned char yiaddr[4];
    unsigned char siaddr[4];
    unsigned char giaddr[4];
    unsigned char chaddr[16];
    unsigned char sname[64];
    unsigned char file[128];
    unsigned int magic_cookie;
    unsigned char options[64];
} dhcp_packet_t;

#define DHCP_OP_REQUEST 1
#define DHCP_OP_REPLY 2

#define DHCP_MAGIC_COOKIE 0x63825363

#define DHCP_DISCOVER 1
#define DHCP_OFFER 2
#define DHCP_REQUEST 3
#define DHCP_ACK 5

void dhcp_init(const unsigned char *our_mac);
int dhcp_request_ip(unsigned char *out_ip);

#endif
