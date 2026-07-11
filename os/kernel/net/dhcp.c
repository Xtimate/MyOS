#include "dhcp.h"
#include "udp.h"
#include "net.h"
#include "../include/framebuffer.h"
#include "../include/pci.h"

#define DHCP_FIXED_HEADER_SIZE 240

static unsigned char my_mac[6];
static volatile int state = 0;       // 0=idle, 1=got offer, 2=got ack
static unsigned char offered_ip[4];
static unsigned char server_ip[4];
static unsigned int  xid = 0x12345678; // fixed for simplicity; a real client randomizes this

void dhcp_init(const unsigned char *our_mac) {
    for (int i = 0; i < 6; i++) my_mac[i] = our_mac[i];
}

static void build_base_packet(dhcp_packet_t *pkt) {
    for (unsigned int i = 0; i < sizeof(dhcp_packet_t); i++) ((unsigned char *)pkt)[i] = 0;

    pkt->op    = DHCP_OP_REQUEST;
    pkt->htype = 1;
    pkt->hlen  = 6;
    pkt->hops  = 0;
    pkt->xid   = htonl(xid);
    pkt->secs  = 0;
    pkt->flags = htons(0x8000); // ask for a broadcast reply, since we have no IP yet to unicast to
    for (int i = 0; i < 6; i++) pkt->chaddr[i] = my_mac[i];
    pkt->magic_cookie = htonl(DHCP_MAGIC_COOKIE);
}

static void send_discover() {
    unsigned char buf[sizeof(dhcp_packet_t)];
    dhcp_packet_t *pkt = (dhcp_packet_t *)buf;
    build_base_packet(pkt);

    int i = 0;
    pkt->options[i++] = 53; pkt->options[i++] = 1; pkt->options[i++] = DHCP_DISCOVER;
    pkt->options[i++] = 55; pkt->options[i++] = 2; pkt->options[i++] = 1; pkt->options[i++] = 3; // request subnet mask, router
    pkt->options[i++] = 255; // end

    unsigned char broadcast[4] = {255, 255, 255, 255};
    udp_send(broadcast, 68, 67, buf, sizeof(dhcp_packet_t));

    fb_terminal_print("DHCP: discover sent\n");
}

static void send_request(const unsigned char *requested_ip, const unsigned char *dhcp_server_ip) {
    unsigned char buf[sizeof(dhcp_packet_t)];
    dhcp_packet_t *pkt = (dhcp_packet_t *)buf;
    build_base_packet(pkt);

    int i = 0;
    pkt->options[i++] = 53; pkt->options[i++] = 1; pkt->options[i++] = DHCP_REQUEST;
    pkt->options[i++] = 50; pkt->options[i++] = 4;
    for (int j = 0; j < 4; j++) pkt->options[i++] = requested_ip[j];
    pkt->options[i++] = 54; pkt->options[i++] = 4;
    for (int j = 0; j < 4; j++) pkt->options[i++] = dhcp_server_ip[j];
    pkt->options[i++] = 255;

    unsigned char broadcast[4] = {255, 255, 255, 255};
    udp_send(broadcast, 68, 67, buf, sizeof(dhcp_packet_t));

    fb_terminal_print("DHCP: request sent\n");
}

// scans the options field for a given option type, returns pointer to its value, or 0 if not found
static unsigned char *find_option(dhcp_packet_t *pkt, unsigned char type, unsigned char *out_len, unsigned int total_len) {
    unsigned char *opt = pkt->options;
    unsigned char *end = (unsigned char *)pkt + total_len; // don't walk past the actual packet

    while (opt < end && *opt != 255) {
        unsigned char opt_type = opt[0];
        unsigned char opt_len  = opt[1];
        if (opt_type == type) {
            if (out_len) *out_len = opt_len;
            return opt + 2;
        }
        opt += 2 + opt_len;
    }
    return 0;
}

static void dhcp_udp_handler(const unsigned char *src_ip, unsigned short src_port,
                              const unsigned char *data, unsigned int len) {
    (void)src_port;

    fb_terminal_print("DHCP handler: len=");
    fb_terminal_print_num(len);
    fb_terminal_print("\n");

    if (len < DHCP_FIXED_HEADER_SIZE) {
        fb_terminal_print("DHCP: packet too short\n");
        return;
    }

    dhcp_packet_t *pkt = (dhcp_packet_t *)data;

    fb_terminal_print("DHCP: xid=");
    pci_print_hex(ntohl(pkt->xid));
    fb_terminal_print(" expected=");
    pci_print_hex(xid);
    fb_terminal_print("\n");

    if (ntohl(pkt->xid) != xid) return; // not our transaction

    unsigned char opt_len;
    unsigned char *msg_type = find_option(pkt, 53, &opt_len, len);
    if (!msg_type) {
        fb_terminal_print("DHCP: no msg_type option found\n");
        return;
    }

    fb_terminal_print("DHCP: msg_type=");
    fb_terminal_print_num(*msg_type);
    fb_terminal_print("\n");

    if (*msg_type == DHCP_OFFER && state == 0) {
        for (int i = 0; i < 4; i++) offered_ip[i] = pkt->yiaddr[i];
        for (int i = 0; i < 4; i++) server_ip[i]  = src_ip[i];
        state = 1;

        fb_terminal_print("DHCP: offer received, ip pending accept\n");

    } else if (*msg_type == DHCP_ACK && state == 1) {
        for (int i = 0; i < 4; i++) offered_ip[i] = pkt->yiaddr[i];
        state = 2;

        fb_terminal_print("DHCP: ack received\n");
    }
}

int dhcp_request_ip(unsigned char *out_ip) {
    state = 0;
    udp_register_handler(68, dhcp_udp_handler); // we listen on port 68 (client port)

    send_discover();

    // wait for offer
    for (int i = 0; i < 3000000 && state == 0; i++) {
        net_poll();
    }
    if (state == 0) {
        fb_terminal_print("DHCP: no offer received (timeout)\n");
        return 0;
    }

    send_request(offered_ip, server_ip);

    // wait for ack
    for (int i = 0; i < 3000000 && state == 1; i++) {
        net_poll();
    }
    if (state != 2) {
        fb_terminal_print("DHCP: no ack received (timeout)\n");
        return 0;
    }

    for (int i = 0; i < 4; i++) out_ip[i] = offered_ip[i];
    return 1;
}
