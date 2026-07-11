#include "icmp.h"
#include "ipv4.h"
#include "../include/framebuffer.h"
#include "net.h"

static volatile int reply_received = 0;
static unsigned short expected_id = 0;
static unsigned short expected_seq = 0;

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

    return (unsigned short)~sum;
}

void icmp_handle_packet(const unsigned char *src_ip, const unsigned char *data, unsigned int len) {
    if (len < sizeof(icmp_header_t)) return;

    icmp_header_t *pkt = (icmp_header_t *)data;

    if (pkt->type == ICMP_ECHO_REQUEST) {
        fb_terminal_print("ICMP: echo request received\n");

        unsigned char reply_buf[1500];
        if (len > sizeof(reply_buf)) return;

        icmp_header_t *reply = (icmp_header_t *)reply_buf;
        reply->type     = ICMP_ECHO_REPLY;
        reply->code     = 0;
        reply->id       = pkt->id;
        reply->seq      = pkt->seq;
        reply->checksum = 0;

        unsigned int payload_len = len - sizeof(icmp_header_t);
        for (unsigned int i = 0; i < payload_len; i++) {
            reply_buf[sizeof(icmp_header_t) + i] = data[sizeof(icmp_header_t) + i];
        }
        reply->checksum = ip_checksum(reply_buf, len);

        ipv4_send(src_ip, IP_PROTO_ICMP, reply_buf, len);
        fb_terminal_print("ICMP: echo reply sent\n");

    } else if (pkt->type == ICMP_ECHO_REPLY) {
        if (ntohs(pkt->id) == expected_id && ntohs(pkt->seq) == expected_seq) {
            reply_received = 1;
        }
    }
}

int icmp_ping(const unsigned char *dest_ip) {
    unsigned char packet[sizeof(icmp_header_t) + 32]; // header + small payload
    icmp_header_t *icmp = (icmp_header_t *)packet;

    expected_id  = 1;
    expected_seq = 1;
    reply_received = 0;

    icmp->type     = ICMP_ECHO_REQUEST;
    icmp->code     = 0;
    icmp->id       = htons(expected_id);
    icmp->seq      = htons(expected_seq);
    icmp->checksum = 0;

    // fill payload with a simple pattern, like real ping does
    for (unsigned int i = 0; i < 32; i++) {
        packet[sizeof(icmp_header_t) + i] = 'a' + (i % 23);
    }

    icmp->checksum = ip_checksum(packet, sizeof(packet));

    fb_terminal_print("ICMP: sending echo request\n");
    ipv4_send(dest_ip, IP_PROTO_ICMP, packet, sizeof(packet));

    // poll until we see the matching reply or give up
    for (int i = 0; i < 2000000; i++) {
        net_poll();
        if (reply_received) {
            fb_terminal_print("ICMP: echo reply received\n");
            return 1;
        }
    }

    fb_terminal_print("ICMP: ping timed out\n");
    return 0;
}
