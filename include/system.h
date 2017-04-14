#pragma once

#include "types.h"

extern uint8 inportb(uint16 port);

extern void outportb(uint16 port, uint8 data);

extern uint16 inportw(uint16 port);

extern void outportw(uint16 port, uint16 data);

extern uint32 inportl(uint16 port);

extern void outportl(uint16 port, uint32 data);

extern void io_wait();

#define asm __asm__
#define volatile __volatile__
