#include "udp.h"
#include "ipv4.h"
#include "../include/framebuffer.h"

#define UDP_MAX_HANDLERS 8

typedef struct {
    unsigned short port;
    udp_handler_t handler;
    int used;
} udp_binding_t;

static udp_binding_t bindings[UDP_MAX_HANDLERS];
static unsigned char my_ip[4];

void udp_init() {
    for (int i = 0; i < UDP_MAX_HANDLERS; i++) bindings[i].used = 0;
}

void udp_register_handler(unsigned short port, udp_handler_t handler) {
    for (int i = 0; i < UDP_MAX_HANDLERS; i++) {
        if (!bindings[i].used) {
            bindings[i].port = port;
            bindings[i].handler = handler;
            bindings[i].used = 1;
            return;
        }
    }
}

static unsigned short ip_checksum(const void *data, unsigned int len) {
    const unsigned short *ptr = (const unsigned short *)data;
    unsigned int sum = 0;
    while (len > 1) { sum += *ptr++; len -= 2; }
    if (len == 1) sum += *(const unsigned char *)ptr;
    while (sum >> 16) sum = (sum & 0xFFFF) + (sum >> 16);
    return (unsigned short)(~sum);
}

typedef struct __attribute__((packed)) {
    unsigned char src_ip[4];
    unsigned char dest_ip[4];
    unsigned char zero;
    unsigned char protocol;
    unsigned short udp_length;
} udp_pseudo_header_t;

int udp_send(const unsigned char *dest_ip, unsigned short src_port, unsigned short dest_port, const unsigned char *data, unsigned int len) {
    unsigned char packet[1500];
    unsigned int total_len = sizeof(udp_header_t) + len;
    if (total_len > sizeof(packet)) return 0;

    udp_header_t *udp = (udp_header_t *)packet;
    udp->src_port = htons(src_port);
    udp->dest_port = htons(dest_port);
    udp->length = htons(total_len);
    udp->checksum = 0;

    unsigned char *payload = packet + sizeof(udp_header_t);
    for (unsigned int i = 0; i < len; i++) payload[i] = data[i];

    unsigned char csum_buf[sizeof(udp_pseudo_header_t) + 1500];
    udp_pseudo_header_t *pseudo = (udp_pseudo_header_t *)csum_buf;
    for (int i = 0; i < 4; i++) pseudo->src_ip[i] = my_ip[i];
    for (int i = 0; i < 4; i++) pseudo->dest_ip[i] = dest_ip[i];
    pseudo->zero = 0;
    pseudo->protocol = IP_PROTO_UDP;
    pseudo->udp_length = htons(total_len);

    for (unsigned int i = 0; i < total_len; i++) {
        csum_buf[sizeof(udp_pseudo_header_t) + i] = packet[i];
    }

    udp->checksum = ip_checksum(csum_buf, sizeof(udp_pseudo_header_t) + total_len);
    if (udp->checksum == 0) udp->checksum = 0xFFFF;

    return ipv4_send(dest_ip, IP_PROTO_UDP, packet, total_len);
}

void udp_handle_packet(const unsigned char *src_ip, const unsigned char *data, unsigned int len) {
    if (len < sizeof(udp_header_t)) return;

    udp_header_t *udp = (udp_header_t *)data;
    unsigned short dest_port = ntohs(udp->dest_port);
    unsigned short src_port = ntohs(udp->src_port);

    unsigned char *payload = (unsigned char *)data + sizeof(udp_header_t);
    unsigned int payload_len = ntohs(udp->length) - sizeof(udp_header_t);

    for (int i = 0; i < UDP_MAX_HANDLERS; i++) {
        if (bindings[i].used && bindings[i].port == dest_port) {
            bindings[i].handler(src_ip, src_port, payload, payload_len);
            return;
        }
    }

    fb_terminal_print("UDP: no handler for port\n");
}

void udp_set_ip(const unsigned char *ip) {
    for (int i = 0; i < 4; i++) my_ip[i] = ip[i];
}
