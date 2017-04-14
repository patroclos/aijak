#include "system.h"

inline uint8 inportb(uint16 _port)
{
	uint8 rv;
	asm volatile("inb %%dx, %%al"
	             : "=a"(rv)
	             : "d"(_port));
	return rv;
}

inline void outportb(uint16 _port, uint8 _data)
{
	asm volatile("outb %%al, %%dx"
	             :
	             : "d"(_port), "a"(_data));
}

inline uint16 inportw(uint16 port)
{
	uint16 rv;
	asm volatile("inw %%dx, %%ax"
	             : "=a"(rv)
	             : "d"(port));
	return rv;
}

inline void outportw(uint16 port, uint16 data)
{
	asm volatile("outw %%ax, %%dx"
	             :
	             : "d"(port), "a"(data));
}

inline uint32 inportl(uint16 port)
{
	uint32 rv;
	asm volatile("inl %%dx, %%eax"
	             : "=a"(rv)
	             : "d"(port));
	return rv;
}

inline void outportl(uint16 port, uint32 data)
{
	asm volatile("outl %%eax, %%dx" ::"d"(port), "a"(data));
}

inline void io_wait()
{
	asm volatile("jmp 1f\n\t"
	             "1:jmp 2f\n\t"
	             "2:");
}
