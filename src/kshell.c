#include <kshell.h>

#include <display/display.h>
#include <drivers/keyboard.h>
#include <memory/memory.h>
#include <net/arp.h>
#include <net/icmp.h>
#include <net/route.h>
#include <string.h>

MODULE("KSH");

void kshell_init()
{
	char *cmdbuf = (char *)malloc(0xFF);
	int   i;

	NetIntf *intf      = (NetIntf *)g_netIntfList.next;
	IPv4Addr routeDest = { { { 0, 0, 0, 0 } } };
	IPv4Addr routeMask = { { { 0, 0, 0, 0 } } };
	IPv4Addr routeGW   = { { { 192, 168, 2, 1 } } };
	net_route_add(&routeDest, &routeMask, &routeGW, intf);
	while (true)
	{
		memset(cmdbuf, 0, 0xFF);
		kprintf("KSH > ");
		for (i = 0; i < 0xFF; i++)
		{
			char ch = 0;
			while (ch == 0)
				ch = keyboard_get_key();

			kprintf("%c", ch);

			if (ch == '\n')
				break;
			cmdbuf[i] = ch;
		}

		size_t splitcount;
		char **split = strtok(cmdbuf, " ", &splitcount);
		if (!strcmp(split[0], "split"))
		{
			size_t s;
			for (s = 0; s < splitcount; s++)
				mprint("%s\n", split[s]);
		}
		if (!strcmp(cmdbuf, "clear"))
		{
			display_clear();
		}

		if (!strcmp(cmdbuf, "arp"))
		{
			arp_dump_entries();
		}

		if (!strcmp(cmdbuf, "ping"))
		{
			IPv4Addr addr;
			addr_strto_ipv4(&addr,split[1]);
			char as[IPV4_ADDR_STRING_SIZE];
			addr_ipv4_tostr(as,IPV4_ADDR_STRING_SIZE,&addr);
			mprint("pinging %s\n", as);
			icmp_echo_request(&addr, 0, 0, 0, 0);
		}
		free(split);
	}
}
