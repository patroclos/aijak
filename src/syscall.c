#include <syscall.h>

#include <interrupt/idt.h>
#include <display/display.h>

MODULE("SYSCALL");

void syscall()
{
	uint32 eax = 0;
	asm volatile("mov %%eax, %0" : "=a"(eax));
	mprint("Received %d\n", eax);
}

void syscall_init()
{
	mprint("Registering syscall interface\n");
	set_idt_gate(0x80, (uint32)syscall);
}
