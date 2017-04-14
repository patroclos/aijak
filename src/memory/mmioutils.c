#include <memory/mmioutils.h>

uint8 mmio_read8(void *p_addr)
{
	return *(volatile uint8*)(p_addr);
}

uint16 mmio_read16(void *p_addr)
{
	return *(volatile uint16*)(p_addr);
}

uint32 mmio_read32(void *p_addr)
{
	return *(volatile uint32*)(p_addr);
}

uint64 mmio_read64(void *p_addr)
{
	return *(volatile uint64*)(p_addr);
}

void mmio_write8(void *p_addr, uint8 p_val)
{
	*(volatile uint8 *)(p_addr) = p_val;
}

void mmio_write16(void *p_addr, uint16 p_val)
{
	*(volatile uint16 *)(p_addr) = p_val;
}

void mmio_write32(void *p_addr, uint32 p_val)
{
	*(volatile uint32 *)(p_addr) = p_val;
}

void mmio_write64(void *p_addr, uint64 p_val)
{
	*(volatile uint64 *)(p_addr) = p_val;
}
