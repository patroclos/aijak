#include <net/addr.h>

#include <stdlib/format.h>

const EthAddr g_nullEthAddr      = { { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };
const EthAddr g_broadcastEthAddr = { { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff } };

const IPv4Addr g_nullIPv4Addr      = { { { 0x00, 0x00, 0x00, 0x00 } } };
const IPv4Addr g_broadcastIPv4Addr = { { { 0xff, 0xff, 0xff, 0xff } } };

bool addr_eth_eq(const EthAddr *x, const EthAddr *y)
{
	uint8 i;
	for (i = 0; i < 6; i++)
		if (x->n[i] != y->n[i])
			return false;
	return true;
}

bool addr_ipv4_eq(const IPv4Addr *x, const IPv4Addr *y)
{
	return x->u.bits == y->u.bits;
}

void addr_eth_tostr(char *str, size_t size, const EthAddr *addr)
{
	snprintf(str, size, "%02x:%02x:%02x:%02x:%02x:%02x", addr->n[0], addr->n[1], addr->n[2], addr->n[3], addr->n[4], addr->n[5]);
}
void addr_ipv4_tostr(char *str, size_t size, const IPv4Addr *addr)
{
	snprintf(str, size, "%d.%d.%d.%d", addr->u.n[0], addr->u.n[1], addr->u.n[2], addr->u.n[3]);
}
void addr_ipv4_port_tostr(char *str, size_t size, const IPv4Addr *addr, uint16 port)
{
	snprintf(str, size, "%d.%d.%d.%d:%d", addr->u.n[0], addr->u.n[1], addr->u.n[2], addr->u.n[3], port);
}

bool addr_strto_ipv4(IPv4Addr *addr, const char *str)
{
	int8 a[4];
	if (sscanf(str, "%d.%d.%d.%d", &a[0], &a[1], &a[2], &a[3]) == 4)
	{
		addr->u.n[0] = a[0];
		addr->u.n[1] = a[1];
		addr->u.n[2] = a[2];
		addr->u.n[3] = a[3];
		return true;
	}
	return false;
}

bool addr_strto_ipv4_port(IPv4Addr *addr, const char *str, uint16 *port)
{
	int8   a[4];
	uint16 n;
	if (sscanf(str, "%d.%d.%d.%d:%d", &a[0], &a[1], &a[2], &a[3], &n) == 4)
	{
		addr->u.n[0] = a[0];
		addr->u.n[1] = a[1];
		addr->u.n[2] = a[2];
		addr->u.n[3] = a[3];
		*port        = n;
		return true;
	}
	return false;
}
