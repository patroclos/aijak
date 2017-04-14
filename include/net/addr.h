#pragma once

#include <types.h>

typedef struct EthAddr
{
	uint8 n[6];
} PACKED EthAddr;

#define ETH_ADDR_STRING_SIZE 18

typedef struct IPv4Addr
{
	union {
		uint8  n[4];
		uint32 bits;
	} u;
} PACKED IPv4Addr;

#define IPV4_ADDR_STRING_SIZE 16
#define IPV4_ADDR_PORT_STRING_SIZE

extern const EthAddr g_nullEthAddr;
extern const EthAddr g_broadcastEthAddr;

extern const IPv4Addr g_nullIPv4Addr;
extern const IPv4Addr g_broadcastIPv4Addr;


bool addr_eth_eq(const EthAddr *x, const EthAddr *y);
bool addr_ipv4_eq(const IPv4Addr *x, const IPv4Addr *y);

void addr_eth_tostr(char *str, size_t size, const EthAddr *addr);
void addr_ipv4_tostr(char *str, size_t size, const IPv4Addr *addr);
void addr_ipv4_port_tostr(char *str, size_t size, const IPv4Addr *addr, uint16 port);

bool addr_strto_ipv4(IPv4Addr *addr, const char *str);
bool addr_strto_ipv4_port(IPv4Addr *addr, const char *str, uint16 *port);
