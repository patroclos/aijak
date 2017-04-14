#include <net/checksum.h>

uint16 net_checksum(const uint8 *data, const uint8 *end)
{
	uint32 sum = net_checksum_acc(data, end, 0);
	return net_checksum_final(sum);
}

uint32 net_checksum_acc(const uint8 *data, const uint8 *end, uint32 sum)
{
	uint32  len = end - data;
	uint16 *p   = (uint16 *)data;

	while (len > 1)
	{
		sum += *p++;
		len -= 2;
	}

	if (len)
		sum += *(uint8 *)p;
	return sum;
}

uint16 net_checksum_final(uint32 sum)
{
	sum = (sum & 0xffff) + (sum >> 16);
	sum += (sum >> 16);

	uint16 temp = ~sum;
	return ((temp & 0x00ff) << 8) | ((temp & 0xff00) >> 8);
}
