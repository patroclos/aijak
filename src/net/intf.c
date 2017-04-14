#include <net/intf.h>

#include <memory/memory.h>

Link g_netIntfList = { &g_netIntfList, &g_netIntfList };

NetIntf *net_intf_create()
{
	NetIntf *intf = (NetIntf *)malloc(sizeof(NetIntf));
	memset(intf, 0, sizeof(NetIntf));
	link_init(&intf->link);

	return intf;
}

void net_intf_add(NetIntf *intf)
{
	link_before(&g_netIntfList, &intf->link);
}
