#pragma once

#include <net/addr.h>
#include <net/buf.h>
#include <link.h>

typedef struct NetIntf
{
	Link link;
	EthAddr ethAddr;
	IPv4Addr ipAddr;
	IPv4Addr broadcastAddr;
	const char *name;

	void (*poll)(struct NetIntf *intf);
	void (*send)(struct NetIntf *intf, const void *dstAddr, uint16 etherType, NetBuf *buf);
	void (*devSend)(NetBuf *buf);
} NetIntf;

extern Link g_netIntfList;

NetIntf *net_intf_create();
void net_intf_add(NetIntf *intf);
