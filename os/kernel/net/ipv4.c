#include "ipv4.h"
#include "arp.h"
#include "../drivers/rtl8139.h"
#include "../include/framebuffer.h"
#include "icmp.h"

static unsigned char my_ip[4];
static unsigned short ip_id_counter = 0;

void ipv4_init(const unsigned char *our_ip) {
    for (int i = 0; i < 4; i++) my_ip[i] = our_ip[i];
}

static unsigned short ip_checksum(const void *data, unsigned int len) {
    const unsigned short *ptr = (const unsigned short *)data;
    unsigned int sum = 0;

    while (len > 1) {
        sum += *ptr++;
        len -= 2;
    }
    if (len == 1) {
        sum += *(const unsigned char *)ptr;
    }

    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return (unsigned short)(~sum);
}

int ipv4_send(const unsigned char *dest_ip, unsigned char protocol, const unsigned char *payload, unsigned int payload_len) {
    unsigned char dest_mac[6];

    unsigned char gateway_ip[4] = {192, 168, 100, 1}; // Hardcoded gateway IP for now
    const unsigned char *arp_target = dest_ip;

    int same_subnet = (dest_ip[0] == my_ip[0] && dest_ip[1] == my_ip[1] && dest_ip[2] == my_ip[2]);
    if (!same_subnet) {
        arp_target = gateway_ip;
    }

    if (!arp_resolve(arp_target, dest_mac)) {
        fb_terminal_print("IPv4: ARP resolve failed\n");
        return 0;
    }

    unsigned int total_len = sizeof(eth_header_t) + sizeof(ipv4_header_t) + payload_len;
    unsigned char frame[1518];
    if (total_len > sizeof(frame)) return 0; // Frame too large

    eth_header_t *eth = (eth_header_t *)frame;
    ipv4_header_t *ip = (ipv4_header_t *)(frame + sizeof(eth_header_t));
    unsigned char *data = frame + sizeof(eth_header_t) + sizeof(ipv4_header_t);

    unsigned char my_mac[6];
    rtl8139_get_mac(my_mac);

    for (int i = 0; i < 6; i++) eth->dest_mac[i] = dest_mac[i];
    for (int i = 0; i < 6; i++) eth->src_mac[i] = my_mac[i];
    eth->ethertype = htons(ETHERTYPE_IPV4);

    ip->ver_ihl = (4 << 4) | 5;
    ip->tos = 0;
    ip->total_len = htons(sizeof(ipv4_header_t) + payload_len);
    ip->id = htons(ip_id_counter++);
    ip->flags_frag = 0;
    ip->ttl = 64;
    ip->protocol = protocol;
    ip->checksum = 0;
    for (int i = 0; i < 4; i++) ip->src_ip[i] = my_ip[i];
    for (int i = 0; i < 4; i++) ip->dest_ip[i] = dest_ip[i];
    ip->checksum = ip_checksum(ip, sizeof(ipv4_header_t));

    for (unsigned int i = 0; i < payload_len; i++) {
        data[i] = payload[i];
    }

    rtl8139_send(frame, total_len);
    return 1;
}

void ipv4_handle_packet(const unsigned char *data, unsigned int len) {
    if (len < sizeof(ipv4_header_t)) return;

    ipv4_header_t *ip = (ipv4_header_t *)data;
    unsigned int header_len = (ip->ver_ihl & 0x0F) * 4;

    if (header_len < sizeof(ipv4_header_t)) return;

    unsigned char *payload = (unsigned char *)data + header_len;
    unsigned int payload_len = ntohs(ip->total_len) - header_len;

    (void)payload;
    (void)payload_len;

    if (ip->protocol == IP_PROTO_ICMP) {
        icmp_handle_packet(ip->src_ip, payload, payload_len);
    }
}
