#include <net/buf.h>

#include <memory/memory.h>

static Link s_netFreeBufs = { &s_netFreeBufs, &s_netFreeBufs };
uint32      g_netBufAllocCount;

NetBuf *net_alloc_buf()
{
	NetBuf *buf;

	if (link_list_is_empty(&s_netFreeBufs))
		buf = (NetBuf *)malloc(NET_BUF_SIZE);
	else
	{
		buf = LinkData(s_netFreeBufs.next, NetBuf, link);
		link_remove(&buf->link);
	}

	buf->link.prev = 0;
	buf->link.next = 0;
	buf->start     = (uint8 *)buf + NET_BUF_START;
	buf->end       = (uint8 *)buf + NET_BUF_START;
	buf->refCount  = 1;

	g_netBufAllocCount++;
	return buf;
}

void net_release_buf(NetBuf *buf)
{
	if (!--buf->refCount)
	{
		--g_netBufAllocCount;
		link_after(&s_netFreeBufs, &buf->link);
		free(buf);
	}
}
