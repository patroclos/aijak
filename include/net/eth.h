#pragma once

#include <net/intf.h>
#include <types.h>

// ethertypes
#define ET_IPV4 0x0800
#define ET_ARP 0x0806
#define ET_IPV6 0x86DD

// ethernet header

typedef struct EthHeader
{
	EthAddr dst;
	EthAddr src;
	uint16  etherType;
} PACKED EthHeader;

// ethernet packet

typedef struct EthPacket
{
	EthHeader *hdr;
	uint16     etherType;
	uint16     hdrLen;
} EthPacket;


// functions

void eth_recv(NetIntf *intf, NetBuf *pkt);
void eth_send_intf(NetIntf *intf, const void *dstAddr, uint16 etherType, NetBuf *pkt);

void eth_print(NetBuf *pkt);
