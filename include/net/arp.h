#pragma once

#include <net/addr.h>
#include <net/buf.h>
#include <net/eth.h>

typedef struct ArpHeader
{
	uint16 htype;
	uint16 ptype;
	uint8  hlen;
	uint8  plen;
	uint16 op;
} PACKED ArpHeader;

void           arp_init();
const EthAddr *arp_lookup_eth_addr(const IPv4Addr *pa);
void arp_request(NetIntf *intf, const IPv4Addr *tpa, uint16 etherType, NetBuf *pkt);
void arp_reply(NetIntf *intf, const EthAddr *tha, const IPv4Addr *tpa);

void arp_recv(NetIntf *intf, NetBuf *pkt);

void arp_dump_entries();
