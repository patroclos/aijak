#include <net/ipv4.h>

#include <display/display.h>
#include <net/checksum.h>
#include <net/eth.h>
#include <net/icmp.h>
#include <net/route.h>
#include <net/swap.h>

MODULE("IPv4");

void ipv4_recv(NetIntf *intf, NetBuf *pkt)
{
	ipv4_print(pkt);

	// validate headers
	if (pkt->start + sizeof(Ipv4Header) > pkt->end)
		return;

	const Ipv4Header *hdr = (const Ipv4Header *)pkt->start;

	uint8 version = hdr->version;
	if (version != 4)
		return;

	uint16 fragment = net_swap16(hdr->offset) & 0x1fff;

	// fragments are not handled yet
	if (fragment)
		return;

	// jump to packet data
	uint8 ihl = hdr->ihl;

	// update packet end
	uint8 *ipEnd = pkt->start + net_swap16(hdr->len);
	if (ipEnd > pkt->end)
	{
		mprint("packet too long\n");
		return;
	}

	pkt->start += ihl << 2;
	pkt->end = ipEnd;

	// dispatch based on protocol
	switch (hdr->protocol)
	{
	case IP_PROTO_ICMP:
		icmp_recv(intf, hdr, pkt);
		break;
	case IP_PROTO_TCP:
		break;
	case IP_PROTO_UDP:
		break;
	}
}

void ipv4_send_intf(NetIntf *intf, const IPv4Addr *nextAddr, const IPv4Addr *dstAddr, uint8 protocol, NetBuf *pkt)
{
	// ipv4 header
	pkt->start -= sizeof(Ipv4Header);

	Ipv4Header *hdr = (Ipv4Header *)pkt->start;
	hdr->version    = 4; // ipv4
	hdr->ihl        = 5; // 5 dwords
	hdr->tos        = 0;
	hdr->len        = net_swap16(pkt->end - pkt->start);
	hdr->id         = net_swap16(0);
	hdr->offset     = net_swap16(0);
	hdr->ttl        = 64;
	hdr->protocol   = protocol;
	hdr->checksum   = 0;
	hdr->src        = intf->ipAddr;
	hdr->dst        = *dstAddr;

	uint16 checksum = net_checksum(pkt->start, pkt->start + sizeof(Ipv4Header));
	hdr->checksum   = net_swap16(checksum);

	ipv4_print(pkt);

	intf->send(intf, nextAddr, ET_IPV4, pkt);
}

void ipv4_send(const IPv4Addr *dstAddr, uint8 protocol, NetBuf *pkt)
{
	const NetRoute *route = net_route_find(dstAddr);

	if (route)
	{
		const IPv4Addr *nextAddr = net_route_next_addr(route, dstAddr);
		ipv4_send_intf(route->intf, nextAddr, dstAddr, protocol, pkt);
	}
}

void ipv4_print(const NetBuf *pkt)
{
	return;
	if (pkt->start + sizeof(IPv4Addr) > pkt->end)
		return;

	const Ipv4Header *hdr = (const Ipv4Header *)pkt->start;

	uint8  version  = hdr->version;
	uint8  ihl      = hdr->ihl;
	uint8  dscp     = (hdr->tos >> 2) & 0x3f;
	uint8  ecn      = (hdr->tos) & 0x3;
	uint16 len      = net_swap16(hdr->len);
	uint16 id       = net_swap16(hdr->id);
	uint16 fragment = net_swap16(hdr->offset) & 0x1fff;
	uint8  ttl      = hdr->ttl;
	uint8  protocol = hdr->protocol;
	uint16 checksum = net_swap16(hdr->checksum);

	uint32 checksum2 = net_checksum(pkt->start, pkt->start + sizeof(Ipv4Header));

	char srcAddrStr[IPV4_ADDR_STRING_SIZE] = { 0 };
	char dstAddrStr[IPV4_ADDR_STRING_SIZE] = { 0 };
	addr_ipv4_tostr(srcAddrStr, IPV4_ADDR_STRING_SIZE, &hdr->src);
	addr_ipv4_tostr(dstAddrStr, IPV4_ADDR_STRING_SIZE, &hdr->dst);

	mprint("version=%d, ihl=%d, dscp=%d, ecn=%d\n", version, ihl, dscp, ecn);
	mprint("len=%d, id=%d, fragment=%d, ttl=%d, protocol=%d, checksum=%d%c\n", len, id, fragment, ttl, protocol, checksum, checksum2 ? '!' : ' ');
	mprint("dst=%s src=%s\n", dstAddrStr, srcAddrStr);
}
