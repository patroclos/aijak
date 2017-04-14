#include <net/eth.h>

#include <display/display.h>
#include <net/addr.h>
#include <net/arp.h>
#include <net/ipv4.h>
#include <net/swap.h>

MODULE("ETH");

bool eth_decode(EthPacket *ep, NetBuf *pkt)
{
	// decode header
	if (pkt->start + sizeof(EthHeader) > pkt->end)
		return false;

	uint8 *    data = pkt->start;
	EthHeader *hdr  = (EthHeader *)data;
	ep->hdr         = hdr;

	// determine which frame type is used
	uint16 n = net_swap16(hdr->etherType);
	if (n <= 1500 && pkt->start + 22 <= pkt->end)
	{
		// 802.2/802.3 encapsulation (RFC 1042)
		uint8 dsap = data[14];
		uint8 ssap = data[15];

		// validate service access point
		if (dsap != 0xaa || ssap != 0xaa)
			return false;

		ep->etherType = (data[20] << 8) | data[21];
		ep->hdrLen    = 22;
	}
	else
	{
		ep->etherType = n;
		ep->hdrLen    = sizeof(EthHeader);
	}
	return true;
}

void eth_recv(NetIntf *intf, NetBuf *pkt)
{
	eth_print(pkt);

	EthPacket ep;
	if (!eth_decode(&ep, pkt))
	{
		// bad packet
		return;
	}

	pkt->start += ep.hdrLen;

	// dispatch packet based on protocol
	switch (ep.etherType)
	{
	case ET_ARP:
		arp_recv(intf, pkt);
		break;
	case ET_IPV4:
		ipv4_recv(intf, pkt);
		break;
	case ET_IPV6:
		break;
	}
}

void eth_send_intf(NetIntf *intf, const void *dstAddr, uint16 etherType, NetBuf *pkt)
{
	// determine ethernet address by protocol of packet
	const EthAddr *dstEthAddr = 0;

	switch (etherType)
	{
	case ET_ARP:
		dstEthAddr = (const EthAddr *)dstAddr;
		break;
	case ET_IPV4:
	{
		const IPv4Addr *dstIpv4Addr = (const IPv4Addr *)dstAddr;
		if (addr_ipv4_eq(dstIpv4Addr, &intf->broadcastAddr))
			dstEthAddr = &g_broadcastEthAddr;
		else
		{
			dstEthAddr = arp_lookup_eth_addr(dstIpv4Addr);
			if (!dstEthAddr)
			{
				arp_request(intf, dstIpv4Addr, etherType, pkt);
				return;
			}
		}
	}
	break;
	case ET_IPV6:
		break;
	}

	if (!dstEthAddr)
	{
		mprint("Dropped packet\n");
		return;
	}

	// fill in ethernet header
	pkt->start -= sizeof(EthHeader);

	EthHeader *hdr = (EthHeader *)pkt->start;
	hdr->dst       = *dstEthAddr;
	hdr->src       = intf->ethAddr;
	hdr->etherType = net_swap16(etherType);

	// transmit
	eth_print(pkt);
	intf->devSend(pkt);
}

void eth_print(NetBuf *pkt)
{
	return;
	EthPacket ep;
	if (eth_decode(&ep, pkt))
	{
		char dstStr[ETH_ADDR_STRING_SIZE] = { 0 };
		char srcStr[ETH_ADDR_STRING_SIZE] = { 0 };

		addr_eth_tostr(dstStr, sizeof(dstStr), &ep.hdr->dst);
		addr_eth_tostr(srcStr, sizeof(srcStr), &ep.hdr->src);

		uint32 len = pkt->end - pkt->start - ep.hdrLen;
		mprint(" dst=%s src=%s et=%x len=%d\n", dstStr, srcStr, ep.etherType, len);
	}
}
