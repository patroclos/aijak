#pragma once

#include <types.h>

typedef struct
{
	uint8  status;
	uint32 size;
} alloc_t;

void mm_init();
void mm_print_out();

void paging_init();
void paging_map_virtual_to_phys(uint32 virt, uint32 phys);

char *pmalloc(size_t size);
char *malloc(size_t size);
void free(void *mem);
void pfree(void *mem);

void *memcpy(const void *dest, const void *src, size_t num);
void *memset(void *ptr, int32 value, size_t num);
void *memset16(void *ptr, uint16 value, size_t num);

