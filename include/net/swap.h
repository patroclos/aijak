#pragma once

#include <types.h>

static inline uint16 net_swap16(uint16 x)
{
	return (x >> 8) | (x << 8);
}

static inline uint32 net_swap32(uint32 x)
{
	union bytes {
		uint8  b[4];
		uint32 n;
	} bytes;
	bytes.n = x;

	return (bytes.b[0] << 24) | (bytes.b[1] << 16) | (bytes.b[2] << 8) | bytes.b[3];
}
