#pragma once

#include <net/intf.h>

typedef struct NetRoute
{
	Link     link;
	IPv4Addr dst;
	IPv4Addr mask;
	IPv4Addr gateway;
	NetIntf *intf;
} NetRoute;

const NetRoute *net_route_find(const IPv4Addr *dst);
void net_route_add(const IPv4Addr *dst, const IPv4Addr *mask, const IPv4Addr *gateway, NetIntf *intf);
const IPv4Addr *net_route_next_addr(const NetRoute *route, const IPv4Addr *dstAddr);
void net_route_print_table();
