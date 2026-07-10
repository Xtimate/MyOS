#ifndef NET_H
#define NET_H

static inline unsigned short htons(unsigned short x) {
    return (x << 8) | (x >> 8);
}
static inline unsigned short ntohs(unsigned short x) { return htons(x); }

static inline unsigned int htonl(unsigned int x) {
    return ((x & 0xFF) << 24) | ((x & 0xFF00) << 8) |
           ((x & 0xFF0000) >> 8) | ((x >> 24) & 0xFF);
}
static inline unsigned int ntohl(unsigned int x) { return htonl(x); }

// Ethernet header — every frame starts with this
typedef struct __attribute__((packed)) {
    unsigned char  dest_mac[6];
    unsigned char  src_mac[6];
    unsigned short ethertype;   // big-endian
} eth_header_t;

#define ETHERTYPE_ARP  0x0806
#define ETHERTYPE_IPV4 0x0800

#endif
