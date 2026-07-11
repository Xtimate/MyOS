#include "arp.h"
#include "../drivers/rtl8139.h"
#include "../include/framebuffer.h"
#include "net.h"

static unsigned char my_mac[6];
static unsigned char my_ip[4];

#define ARP_CACHE_SIZE 16

typedef struct {
    unsigned char ip[4];
    unsigned char mac[6];
    int valid;
} arp_cache_entry_t;

static arp_cache_entry_t arp_cache[ARP_CACHE_SIZE];
static int arp_cache_next = 0; // simple round-robin slot for insertion

static int ip_equal(const unsigned char *a, const unsigned char *b) {
    for (int i = 0; i < 4; i++) if (a[i] != b[i]) return 0;
    return 1;
}

int arp_lookup(const unsigned char *ip, unsigned char *out_mac) {
    for (int i = 0; i < ARP_CACHE_SIZE; i++) {
        if (arp_cache[i].valid && ip_equal(arp_cache[i].ip, ip)) {
            for (int j = 0; j < 6; j++) out_mac[j] = arp_cache[i].mac[j];
            return 1;
        }
    }
    return 0;
}

static void arp_cache_insert(const unsigned char *ip, const unsigned char *mac) {
    // overwrite an existing entry for this IP if present
    for (int i = 0; i < ARP_CACHE_SIZE; i++) {
        if (arp_cache[i].valid && ip_equal(arp_cache[i].ip, ip)) {
            for (int j = 0; j < 6; j++) arp_cache[i].mac[j] = mac[j];
            return;
        }
    }
    // otherwise insert into the next round-robin slot
    arp_cache_entry_t *e = &arp_cache[arp_cache_next];
    for (int j = 0; j < 4; j++) e->ip[j]  = ip[j];
    for (int j = 0; j < 6; j++) e->mac[j] = mac[j];
    e->valid = 1;
    arp_cache_next = (arp_cache_next + 1) % ARP_CACHE_SIZE;
}

void arp_init(const unsigned char *our_mac, const unsigned char *our_ip) {
    for (int i = 0; i < 6; i++) my_mac[i] = our_mac[i];
    for (int i = 0; i < 4; i++) my_ip[i]  = our_ip[i];
    for (int i = 0; i < ARP_CACHE_SIZE; i++) arp_cache[i].valid = 0;
}

static void print_ip(const unsigned char *ip) {
    for (int i = 0; i < 4; i++) {
        fb_terminal_print_num(ip[i]);
        if (i < 3) fb_terminal_print(".");
    }
}

static void print_mac(const unsigned char *mac) {
    const char *hexchars = "0123456789abcdef";
    for (int i = 0; i < 6; i++) {
        char buf[3];
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

    for (int i = 0; i < 6; i++) eth->dest_mac[i] = 0xFF;
    for (int i = 0; i < 6; i++) eth->src_mac[i]  = my_mac[i];
    eth->ethertype = htons(ETHERTYPE_ARP);

    arp->htype = htons(1);
    arp->ptype = htons(0x0800);
    arp->hlen  = 6;
    arp->plen  = 4;
    arp->oper  = htons(ARP_REQUEST);
    for (int i = 0; i < 6; i++) arp->send_mac[i] = my_mac[i];
    for (int i = 0; i < 4; i++) arp->sender_ip[i]  = my_ip[i];
    for (int i = 0; i < 6; i++) arp->target_mac[i] = 0x00;
    for (int i = 0; i < 4; i++) arp->target_ip[i]  = target_ip[i];

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
        arp_cache_insert(arp->sender_ip, arp->send_mac);

        fb_terminal_print("ARP: reply from ");
        print_ip(arp->sender_ip);
        fb_terminal_print(" is at ");
        print_mac(arp->send_mac);
        fb_terminal_print("\n");

    } else if (oper == ARP_REQUEST) {
        // is this request asking about our own IP?
        int is_for_us = 1;
        for (int i = 0; i < 4; i++) {
            if (arp->target_ip[i] != my_ip[i]) { is_for_us = 0; break; }
        }
        if (!is_for_us) return;

        // also worth caching the requester's info while we're at it —
        // we now know their IP/MAC too, for free
        arp_cache_insert(arp->sender_ip, arp->send_mac);

        // build and send an ARP reply
        unsigned char frame[sizeof(eth_header_t) + sizeof(arp_packet_t)];
        eth_header_t *eth = (eth_header_t *)frame;
        arp_packet_t *reply = (arp_packet_t *)(frame + sizeof(eth_header_t));

        for (int i = 0; i < 6; i++) eth->dest_mac[i] = arp->send_mac[i]; // reply directly to requester
        for (int i = 0; i < 6; i++) eth->src_mac[i]  = my_mac[i];
        eth->ethertype = htons(ETHERTYPE_ARP);

        reply->htype = htons(1);
        reply->ptype = htons(0x0800);
        reply->hlen  = 6;
        reply->plen  = 4;
        reply->oper  = htons(ARP_REPLY);
        for (int i = 0; i < 6; i++) reply->send_mac[i] = my_mac[i];
        for (int i = 0; i < 4; i++) reply->sender_ip[i]  = my_ip[i];
        for (int i = 0; i < 6; i++) reply->target_mac[i] = arp->send_mac[i];
        for (int i = 0; i < 4; i++) reply->target_ip[i]  = arp->sender_ip[i];

        rtl8139_send(frame, sizeof(frame));

        fb_terminal_print("ARP: replied to request from ");
        print_ip(arp->sender_ip);
        fb_terminal_print("\n");
    }
}

int arp_resolve(const unsigned char *ip, unsigned char *out_mac) {
    if (arp_lookup(ip, out_mac)) {
        return 1; // already cached, no network round-trip needed
    }

    arp_send_request(ip);

    // poll until it resolves or we time out
    for (int i = 0; i < 2000000; i++) {
        net_poll();
        if (arp_lookup(ip, out_mac)) {
            return 1;
        }
    }

    for (int j = 0; j < 6; j++) out_mac[j] = 0;
    return 0; // timed out
}
