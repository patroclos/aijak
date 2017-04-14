#include <memory/memory.h>
#include <mutex.h>

MUTEX_DEFINE(m_memcpy);

void *memcpy(const void *dest, const void *src, size_t num)
{
	mutex_lock(&m_memcpy);
	char *dst8 = (char *)dest;
	char *src8 = (char *)src;

	if (num & 1)
	{
		dst8[0] = src8[0];
		dst8 += 1;
		src8 += 1;
	}

	num /= 2;
	while (num--)
	{
		dst8[0] = src8[0];
		dst8[1] = src8[1];
		dst8 += 2;
		src8 += 2;
	}
	mutex_unlock(&m_memcpy);
	return (void *)dest;
}

void *memset16(void *ptr, uint16 value, size_t num)
{
	uint16 *p = ptr;
	while (num--)
		*p++ = value;
	return ptr;
}

void *memset(void *ptr, int32 value, size_t num)
{
	unsigned char *p = ptr;
	while (num--)
		*p++ = (unsigned char)value;
	return ptr;
}

