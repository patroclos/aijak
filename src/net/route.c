#include <net/route.h>

#include <memory/memory.h>
#include <display/display.h>
#include <string.h>

MODULE("ROUTE");

static Link s_routeTable = { &s_routeTable, &s_routeTable };

const NetRoute *net_route_find(const IPv4Addr *dst)
{
	NetRoute *route;
	LinkListForEach(route, s_routeTable, link)
	{
		if ((dst->u.bits & route->mask.u.bits) == route->dst.u.bits)
			return route;
	}

	mprint("Failed to route IPv4 address\n");
	return NULL;
}

void net_route_add(const IPv4Addr *dst, const IPv4Addr *mask, const IPv4Addr *gateway, NetIntf *intf)
{
	NetRoute *route = (NetRoute *)malloc(sizeof(NetRoute));
	link_init(&route->link);
	route->dst  = *dst;
	route->mask = *mask;
	if (gateway)
		route->gateway = *gateway;
	else
		route->gateway.u.bits = 0;

	route->intf = intf;

	// insert route at appropriate priority in the table
	NetRoute *prev;
	LinkListForEach(prev, s_routeTable, link)
	{
		if (prev->mask.u.bits > mask->u.bits)
			break;
	}

	link_after(&prev->link, &route->link);
}

const IPv4Addr *net_route_next_addr(const NetRoute *route, const IPv4Addr *dstAddr)
{
	return route->gateway.u.bits ? &route->gateway : dstAddr;
}

void net_route_print_table()
{
	// TODO
}
