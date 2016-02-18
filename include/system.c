#include "system.h"

inline uint8 inportb(uint16 _port)
{
    uint8 rv;
    __asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

inline void outportb(uint16 _port, uint8 _data)
{
    __asm__ __volatile__ ("outb %1, %0" : : "dN" (_port), "a" (_data));
}

inline void io_wait()
{
    __asm__ __volatile__ ("jmp 1f\n\t" "1:jmp 2f\n\t" "2:");
}
