#include <net/arp.h>

#include <display/display.h>
#include <memory/memory.h>
#include <net/swap.h>

MODULE("ARP");

// arp protocol
#define ARP_HTYPE_ETH 0x01

#define ARP_OP_REQUEST 0x01
#define ARP_OP_REPLY 0x02

// arp cache
#define ARP_CACHE_SIZE 16

typedef struct ArpEntry
{
	EthAddr  ha;
	IPv4Addr pa;

	// deferred packet to send
	NetIntf *intf;
	uint16   etherType;
	NetBuf * pkt;
} ArpEntry;

ArpEntry arpCache[ARP_CACHE_SIZE];

void arp_print(const NetBuf *pkt)
{
	return;
	if (pkt->start + sizeof(ArpHeader) > pkt->end)
		return;

	const uint8 *    data = pkt->start;
	const ArpHeader *hdr  = (const ArpHeader *)data;

	uint16 htype = net_swap16(hdr->htype);
	uint16 ptype = net_swap16(hdr->ptype);
	uint8  hlen  = hdr->hlen;
	uint8  plen  = hdr->plen;
	uint16 op    = net_swap16(hdr->op);

	mprint(" htype:0x%x ptype=0x%x hlen=%d plen=%d op=%d\n", htype, ptype, hlen, plen, op);

	if (htype == ARP_HTYPE_ETH && ptype == ET_IPV4 && pkt->start + 28 <= pkt->end)
	{
		const EthAddr * sha = (const EthAddr *)(data + 8);
		const IPv4Addr *spa = (const IPv4Addr *)(data + 14);
		const EthAddr * tha = (const EthAddr *)(data + 18);
		const IPv4Addr *tpa = (const IPv4Addr *)(data + 24);

		char shaStr[ETH_ADDR_STRING_SIZE]  = { 0 };
		char spaStr[IPV4_ADDR_STRING_SIZE] = { 0 };
		char thaStr[ETH_ADDR_STRING_SIZE]  = { 0 };
		char tpaStr[IPV4_ADDR_STRING_SIZE] = { 0 };

		addr_eth_tostr(shaStr, sizeof(shaStr), sha);
		addr_ipv4_tostr(spaStr, sizeof(spaStr), spa);
		addr_eth_tostr(thaStr, sizeof(thaStr), tha);
		addr_ipv4_tostr(tpaStr, sizeof(tpaStr), tpa);

		mprint(" %s spa=%s\n %s tpa=%s\n", shaStr, spaStr, thaStr, tpaStr);
	}
}

void arp_send(NetIntf *intf, uint32 op, const EthAddr *tha, const IPv4Addr *tpa)
{
	NetBuf *pkt  = net_alloc_buf();
	uint8 * data = pkt->start;

	// htype
	data[0] = (ARP_HTYPE_ETH >> 8) & 0xff;
	data[1] = (ARP_HTYPE_ETH)&0xff;

	// ptype
	data[2] = (ET_IPV4 >> 8) & 0xff;
	data[3] = (ET_IPV4)&0xff;

	// hlen
	data[4] = sizeof(EthAddr);

	// plen
	data[5] = sizeof(IPv4Addr);

	// operation
	data[6] = (op >> 8) & 0xff;
	data[7] = (op)&0xff;

	// sha
	*(EthAddr *)(data + 8) = intf->ethAddr;

	// spa
	*(IPv4Addr *)(data + 14) = intf->ipAddr;

	// tha
	if (op == ARP_OP_REQUEST)
		*(EthAddr *)(data + 18) = g_nullEthAddr;
	else
		*(EthAddr *)(data + 18) = *tha;

	// tpa
	*(IPv4Addr *)(data + 24) = *tpa;

	pkt->end += 28;

	//transmit packet
	arp_print(pkt);
	intf->send(intf, tha, ET_ARP, pkt);
}

ArpEntry *arp_lookup(const IPv4Addr *pa)
{
	ArpEntry *entry = arpCache;
	ArpEntry *end   = entry + ARP_CACHE_SIZE;

	for (; entry != end; entry++)
		if (addr_ipv4_eq(&entry->pa, pa))
			return entry;
	return 0;
}

ArpEntry *arp_add(const EthAddr *ha, const IPv4Addr *pa)
{
	// TODO handle overflow
	ArpEntry *entry = arpCache;
	ArpEntry *end   = entry + ARP_CACHE_SIZE;
	for (; entry != end; entry++)
		if (!entry->pa.u.bits)
			break;

	if (entry != end)
	{
		entry->ha = *ha;
		entry->pa = *pa;
		return entry;
	}
	mprint("Ran out of ARP entries\n");
	return 0;
}

void arp_request(NetIntf *intf, const IPv4Addr *tpa, uint16 etherType, NetBuf *pkt)
{
	ArpEntry *entry = arp_lookup(tpa);
	if (!entry)
	{
		entry = arp_add(&g_nullEthAddr, tpa);
	}

	if (entry)
	{
		if (entry->pkt)
			net_release_buf(entry->pkt);

		entry->intf      = intf;
		entry->etherType = etherType;
		entry->pkt       = pkt;
		arp_send(intf, ARP_OP_REQUEST, &g_broadcastEthAddr, tpa);
	}
}

void arp_reply(NetIntf *intf, const EthAddr *tha, const IPv4Addr *tpa)
{
	arp_send(intf, ARP_OP_REPLY, tha, tpa);
}

void arp_init()
{
	ArpEntry *entry = arpCache;
	ArpEntry *end   = entry + ARP_CACHE_SIZE;
	for (; entry != end; entry++)
	{
		memset(&entry->ha, 0, sizeof(EthAddr));
		memset(&entry->pa, 0, sizeof(IPv4Addr));
	}
}

const EthAddr *arp_lookup_eth_addr(const IPv4Addr *pa)
{
	ArpEntry *entry = arp_lookup(pa);
	if (entry)
		return &entry->ha;
	return 0;
}

void arp_recv(NetIntf *intf, NetBuf *pkt)
{
	arp_print(pkt);

	// decode header
	if (pkt->start + sizeof(ArpHeader) > pkt->end)
		return;

	const uint8 *    data = pkt->start;
	const ArpHeader *hdr  = (const ArpHeader *)data;

	uint16 htype = net_swap16(hdr->htype);
	uint16 ptype = net_swap16(hdr->ptype);
	uint16 op    = net_swap16(hdr->op);

	// skip packets that are not ethernet, ipv4 or well formed
	if (htype != ARP_HTYPE_ETH || ptype != ET_IPV4 || pkt->start + 28 > pkt->end)
		return;

	// decode addresses
	const EthAddr * sha = (const EthAddr *)(data + 8);
	const IPv4Addr *spa = (const IPv4Addr *)(data + 14);
	//const EthAddr *tha =(const EthAddr*)(data+18);
	const IPv4Addr *tpa = (const IPv4Addr *)(data + 24);

	// update existing entry if we know about this source ip address
	bool      merge = false;
	ArpEntry *entry = arp_lookup(spa);
	if (entry)
	{
		entry->ha = *sha;
		merge     = true;

		// send deferrred packet
		if (entry->pkt)
		{
			eth_send_intf(entry->intf, spa, entry->etherType, entry->pkt);
			entry->intf      = 0;
			entry->etherType = 0;
			entry->pkt       = 0;
		}
	}

	// check if arp packet is targeting our ip
	if (addr_ipv4_eq(tpa, &intf->ipAddr))
	{
		if (!merge)
		{
			arp_add(sha, spa);
		}

		if (op == ARP_OP_REQUEST)
		{
			char hastr[ETH_ADDR_STRING_SIZE]  = { 0 };
			char pastr[IPV4_ADDR_STRING_SIZE] = { 0 };
			addr_eth_tostr(hastr, ETH_ADDR_STRING_SIZE, sha);
			addr_ipv4_tostr(pastr, IPV4_ADDR_STRING_SIZE, spa);
			arp_reply(intf, sha, spa);
		}
	}
}

void arp_dump_entries()
{
	ArpEntry *entry = arpCache;
	ArpEntry *end   = entry + ARP_CACHE_SIZE;
	for (; entry != end; entry++)
	{
		if (!entry->pa.u.bits)
			continue;

		char ha[ETH_ADDR_STRING_SIZE]  = { 0 };
		char pa[IPV4_ADDR_STRING_SIZE] = { 0 };

		addr_eth_tostr(ha, ETH_ADDR_STRING_SIZE, &entry->ha);
		addr_ipv4_tostr(pa, IPV4_ADDR_STRING_SIZE, &entry->pa);

		mprint("#%d MAC=%s IP=%s\n", entry - arpCache, ha, pa);
	}
}
