#include "net.h"
#include "arp.h"
#include "../drivers/rtl8139.h"

void net_poll() {
    unsigned char buf[1518];
    unsigned int len = rtl8139_receive(buf);
    if (len == 0) return;

    if (len < sizeof(eth_header_t)) return;

    eth_header_t *eth = (eth_header_t *)buf;
    unsigned short ethertype = ntohs(eth->ethertype);

    if (ethertype == ETHERTYPE_ARP) {
        arp_handle_packet(buf + sizeof(eth_header_t), len - sizeof(eth_header_t));
    }
}
