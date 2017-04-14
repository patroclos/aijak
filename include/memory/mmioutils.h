#pragma once

#include <types.h>

uint8 mmio_read8(void *p_addr);
uint16 mmio_read16(void *p_addr);
uint32 mmio_read32(void *p_addr);
uint64 mmio_read64(void *p_addr);

void mmio_write8(void *p_addr, uint8 p_val);
void mmio_write16(void *p_addr, uint16 p_val);
void mmio_write32(void *p_addr, uint32 p_val);
void mmio_write64(void *p_addr, uint64 p_val);
