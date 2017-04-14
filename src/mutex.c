#include <display/display.h>

#include <mutex.h>
#include <tasking.h>

void mutex_lock(mutex *m)
{
	uint8 retry = 1;

	while (retry)
	{
		asm volatile("xor %eax, %eax; xor %ebx, %ebx; inc %ebx");
		asm volatile("lock; cmpxchg %%ebx, %0"
		             :
		             : "m"(m->locked));

		asm volatile("mov %%al, %0"
		             : "=a"(retry));

		schedule_noirq();
	}
}

void mutex_unlock(mutex *m)
{
	m->locked = 0;
	schedule_noirq();
}
