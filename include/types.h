#pragma once

typedef signed char   int8;
typedef unsigned char uint8;

typedef signed short   int16;
typedef unsigned short uint16;

typedef signed int   int32;
typedef unsigned int uint32;
typedef unsigned int size_t;

typedef signed long long   int64;
typedef unsigned long long uint64;

typedef enum { false = 0,
	true             = !false } bool;

typedef uint64 uintptr_t;

#define low_16(address) (uint16)((address)&0xFFFF)
#define high_16(address) (uint16)(((address) >> 16) & 0xFFFF)

#define NULL 0UL

#define PACKED __attribute__((packed))

/* Registers */
struct regs
{
	unsigned int gs, fs, es, ds;
	unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
	unsigned int int_no, err_code;
	unsigned int eip, cs, eflags, useresp, ss;
};
