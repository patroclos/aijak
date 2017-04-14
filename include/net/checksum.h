#pragma once

#include <net/addr.h>

typedef struct ChecksumHeader
{
	IPv4Addr src;
	IPv4Addr dst;
	uint8 reserved;
	uint8 protocol;
	uint16 len;
} PACKED ChecksumHeader;

uint16 net_checksum(const uint8 *data, const uint8 *end);
uint32 net_checksum_acc(const uint8 *data, const uint8 *end, uint32 sum);
uint16 net_checksum_final(uint32 sum);
