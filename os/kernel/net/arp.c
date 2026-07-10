#include "arp.h"
#include "../drivers/rtl8139.h"
#include "../include/framebuffer.h"

static unsigned char my_mac[6];
static unsigned char my_ip[4];

void arp_init(const unsigned char *our_mac, const unsigned char *our_ip) {
    for (int i = 0; i < 6; i++) my_mac[i] = our_mac[i];
    for (int i = 0; i < 4; i++) my_ip[i] = our_ip[i];
}

static void print_ip(const unsigned char *ip) {
    for (int i = 0; i < 4; i++) {
        fb_terminal_print_num(ip[i]);
        if (i < 3) fb_terminal_print(".");
    }
}

static void print_mac(const unsigned char *mac) {
    for (int i = 0; i < 6; i++) {
        char buf[3];
        const char *hexchars = "0123456789abcdef";
        buf[0] = hexchars[(mac[i] >> 4) & 0xF];
        buf[1] = hexchars[mac[i] & 0xF];
        buf[2] = 0;
        fb_terminal_print(buf);
        if (i < 5) fb_terminal_print(":");
    }
}

void arp_send_request(const unsigned char *target_ip) {
    unsigned char frame[sizeof(eth_header_t) + sizeof(arp_packet_t)];

    eth_header_t *eth = (eth_header_t *)frame;
    arp_packet_t *arp = (arp_packet_t *)(frame + sizeof(eth_header_t));

    for (int i = 0; i < 6; i++) eth->dest_mac[i] = 0xFF; // broadcast
    for (int i = 0; i < 6; i++) eth->src_mac[i] = my_mac[i];
    eth->ethertype = htons(ETHERTYPE_ARP);

    arp->htype = htons(1); // Ethernet
    arp->ptype = htons(0x0800); // IPv4
    arp->hlen = 6;
    arp->plen = 4;
    arp->oper = htons(ARP_REQUEST);
    for (int i = 0; i < 6; i++) arp->send_mac[i] = my_mac[i];
    for (int i = 0; i < 4; i++) arp->sender_ip[i] = my_ip[i];
    for (int i = 0; i < 6; i++) arp->target_mac[i] = 0x00;
    for (int i = 0; i < 4; i++) arp->target_ip[i] = target_ip[i];

    rtl8139_send(frame, sizeof(frame));

    fb_terminal_print("ARP: request sent for ");
    print_ip(target_ip);
    fb_terminal_print("\n");
}

void arp_handle_packet(const unsigned char *data, unsigned int len) {
    if (len < sizeof(arp_packet_t)) return;

    arp_packet_t *arp = (arp_packet_t *)data;
    unsigned short oper = ntohs(arp->oper);

    if (oper == ARP_REPLY) {
        fb_terminal_print("ARP: reply from ");
        print_ip(arp->sender_ip);
        fb_terminal_print(" with MAC ");
        print_mac(arp->send_mac);
        fb_terminal_print("\n");
    }
}
