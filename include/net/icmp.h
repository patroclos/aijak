#pragma once

#include <net/ipv4.h>

void icmp_recv(NetIntf *intf, const Ipv4Header *ipHdr, NetBuf *pkt);

void icmp_echo_request(const IPv4Addr *dstAddr,uint16 id, uint16 sequence,const uint8 *data, const uint8 *end);
