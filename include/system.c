#include "system.h"

inline uint8 inportb(uint16 _port)
{
    uint8 rv;
    asm volatile ("inb %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

inline void outportb(uint16 _port, uint8 _data)
{
    asm volatile ("outb %1, %0" : : "dN" (_port), "a" (_data));
}

inline void io_wait()
{
    asm volatile ("jmp 1f\n\t" "1:jmp 2f\n\t" "2:");
}
