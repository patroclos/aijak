#pragma once

#include <net/intf.h>
#include <net/addr.h>
#include <types.h>

#define IP_PROTO_ICMP 1
#define IP_PROTO_TCP 6
#define IP_PROTO_UDP 17

typedef struct Ipv4Header
{
	uint8 ihl:4;
	uint8 version:4;
	uint8 tos;
	uint16 len;
	uint16 id;
	uint16 offset;
	uint8 ttl;
	uint8 protocol;
	uint16 checksum;
	IPv4Addr src;
	IPv4Addr dst;
} PACKED Ipv4Header;

void ipv4_recv(NetIntf *intf, NetBuf *pkt);
void ipv4_send(const IPv4Addr *dstAddr, uint8 protocol, NetBuf *pkt);
void ipv4_send_intf(NetIntf *intf, const IPv4Addr *nextAddr, const IPv4Addr *dstAddr, uint8 protocol, NetBuf *pkt);

void ipv4_print(const NetBuf *pkt);
