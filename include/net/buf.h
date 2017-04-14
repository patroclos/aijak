#pragma once

#include <types.h>
#include <link.h>

#define NET_BUF_SIZE 2048
#define NET_BUF_START 256

typedef struct NetBuf
{
	Link link;
	uint8 *start; // offset to data start
	uint8 *end; // offsrt to data end exclusive
	uint32 refCount;
	uint32 seq; // data from TCP header used for out-of-order/retransmit
	uint8 flags; // data from TCP header used for out-of-order/retransmit
} NetBuf;

NetBuf *net_alloc_buf();
void net_release_buf(NetBuf *buf);
