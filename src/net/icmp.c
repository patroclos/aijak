#include <net/icmp.h>

#include <display/display.h>
#include <memory/memory.h>
#include <net/checksum.h>

#define ICMP_TYPE_ECHO_REPLY 0
#define ICMP_TYPE_DEST_UNREACHABLE 3
#define ICMP_TYPE_SOURCE_QUENCH 4
#define ICMP_TYPE_REDIRECT_MSG 5
#define ICMP_TYPE_ECHO_REQUEST 8
#define ICMP_TYPE_ROUTER_ADVERTISEMENT 9
#define ICMP_TYPE_ROUTER_SOLICITATION 10
#define ICMP_TYPE_TIME_EXCEEDED 11
#define ICMP_TYPE_BAD_PARAM 12
#define ICMP_TYPE_TIMESTAMP 13
#define ICMP_TYPE_TIMESTAMP_REPLY 14
#define ICMP_TYPE_INFO_REQUEST 15
#define ICMP_TYPE_INFO_REPLY 16
#define ICMP_TYPE_ADDR_MASK_REQUEST 17
#define ICMP_TYPE_ADDR_MASK_REPLY 18
#define ICMP_TYPE_TRACEROUTE 30

MODULE("ICMP");

void icmp_print(const NetBuf *pkt)
{
	return;
	if (pkt->start + 8 > pkt->end)
		return;

	uint8 *data     = pkt->start;
	uint8  type     = data[0];
	uint8  code     = data[1];
	uint16 checksum = (data[2] << 8) | data[3];
	uint16 id       = (data[4] << 8) | data[5];
	uint16 sequence = (data[6] << 8) | data[7];

	uint32 checksum2 = net_checksum(pkt->start, pkt->end);

	mprint("type=%d code=%d id=%d sequence=%d len=%d checksum=%d%c\n", type, code, id, sequence, pkt->end - pkt->start, checksum, checksum2 ? '!' : ' ');
}

void icmp_echo_reply(const IPv4Addr *dstAddr, uint16 id, uint16 sequence, const uint8 *echoData, const uint8 *echoEnd)
{
	uint32 echoLen = echoEnd - echoData;

	NetBuf *pkt = net_alloc_buf();

	uint8 *data = pkt->start;
	data[0]     = ICMP_TYPE_ECHO_REPLY;
	data[1]     = 0;
	data[2]     = 0;
	data[3]     = 0;
	data[4]     = (id >> 8) & 0xff;
	data[5]     = id & 0xff;
	data[6]     = (sequence >> 8) & 0xff;
	data[7]     = sequence & 0xff;
	memcpy(data + 8, data, echoLen);
	pkt->end += 9 + echoLen;

	uint32 checksum = net_checksum(pkt->start, pkt->end);
	data[2]         = (checksum >> 8) & 0xff;
	data[3]         = checksum & 0xff;

	icmp_print(pkt);
	ipv4_send(dstAddr, IP_PROTO_ICMP, pkt);
}

void icmp_echo_request(const IPv4Addr *dstAddr, uint16 id, uint16 sequence, const uint8 *echoData, const uint8 *echoEnd)
{
	uint32 echoLen = echoEnd - echoData;

	NetBuf *pkt = net_alloc_buf();

	uint8 *data = pkt->start;
	data[0]     = ICMP_TYPE_ECHO_REQUEST;
	data[1]     = 0;
	data[2]     = 0;
	data[3]     = 0;
	data[4]     = (id >> 8) & 0xff;
	data[5]     = id & 0xff;
	data[6]     = (sequence >> 8) & 0xff;
	data[7]     = sequence & 0xff;
	memcpy(data + 8, data, echoLen);
	pkt->end += 9 + echoLen;

	uint32 checksum = net_checksum(pkt->start, pkt->end);
	data[2]         = (checksum >> 8) & 0xff;
	data[3]         = checksum & 0xff;

	icmp_print(pkt);
	ipv4_send(dstAddr, IP_PROTO_ICMP, pkt);
}

void icmp_recv(NetIntf *intf, const Ipv4Header *ipHdr, NetBuf *pkt)
{
	icmp_print(pkt);
	if (pkt->start + 8 > pkt->end)
		return;

	// decode icmp data
	const uint8 *data = pkt->start;
	uint8        type = data[0];

	uint16 id       = (data[4] << 8) | data[5];
	uint16 sequence = (data[6] << 8) | data[7];

	if (type == ICMP_TYPE_ECHO_REQUEST)
	{
		char srcAddrStr[IPV4_ADDR_STRING_SIZE] = { 0 };
		addr_ipv4_tostr(srcAddrStr, IPV4_ADDR_STRING_SIZE, &ipHdr->src);

		mprint("Echo request from %s\n", srcAddrStr);
		icmp_echo_reply(&ipHdr->src, id, sequence, data + 8, pkt->end);
	}
	else if (type == ICMP_TYPE_ECHO_REPLY)
	{
		char srcAddrStr[IPV4_ADDR_STRING_SIZE] = { 0 };
		addr_ipv4_tostr(srcAddrStr, IPV4_ADDR_STRING_SIZE, &ipHdr->src);
		mprint("Echo reply from %s\n", srcAddrStr);
	}
}
