#pragma once

#include "types.h"

extern inline uint8 inportb(uint16 _port);

extern inline void outportb(uint16 _port, uint8 _data);

extern inline void io_wait();

#define asm __asm__
#define volatile __volatile__
