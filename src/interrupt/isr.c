#include <interrupt/idt.h>

#include <display/display.h>
#include <interrupt/irq.h>
#include <interrupt/isr.h>
#include <signal.h>
#include <tasking.h>
#include <util.h>

void idt_init()
{
	set_idt_gate(0, (uint32)_isr0);
	set_idt_gate(1, (uint32)_isr1);
	set_idt_gate(2, (uint32)_isr2);
	set_idt_gate(3, (uint32)_isr3);
	set_idt_gate(4, (uint32)_isr4);
	set_idt_gate(5, (uint32)_isr5);
	set_idt_gate(6, (uint32)_isr6);
	set_idt_gate(7, (uint32)_isr7);
	set_idt_gate(8, (uint32)_isr8);
	set_idt_gate(9, (uint32)_isr9);
	set_idt_gate(10, (uint32)_isr10);
	set_idt_gate(11, (uint32)_isr11);
	set_idt_gate(12, (uint32)_isr12);
	set_idt_gate(13, (uint32)_isr13);
	set_idt_gate(14, (uint32)_isr14);
	set_idt_gate(15, (uint32)_isr15);
	set_idt_gate(16, (uint32)_isr16);
	set_idt_gate(17, (uint32)_isr17);
	set_idt_gate(18, (uint32)_isr18);
	set_idt_gate(19, (uint32)_isr19);
	set_idt_gate(20, (uint32)_isr20);
	set_idt_gate(21, (uint32)_isr21);
	set_idt_gate(22, (uint32)_isr22);
	set_idt_gate(23, (uint32)_isr23);
	set_idt_gate(24, (uint32)_isr24);
	set_idt_gate(25, (uint32)_isr25);
	set_idt_gate(26, (uint32)_isr26);
	set_idt_gate(27, (uint32)_isr27);
	set_idt_gate(28, (uint32)_isr28);
	set_idt_gate(29, (uint32)_isr29);
	set_idt_gate(30, (uint32)_isr30);
	set_idt_gate(31, (uint32)_isr31);
	set_idt();
}

void isr_handler(struct regs *r)
{

	if (is_tasking())
	{
		switch (r->int_no)
		{
		case 0:
			send_sig(SIGILL);
			break;
		case 13:
			send_sig(SIGSEGV);
			break;
		}
		return;
	}

	kprintf("ISR %d called\n", r->int_no);
	kprintf("%s\n", exception_messages[r->int_no]);

	if (r->int_no == 14) // page fault
	{
		uint32 err = 0;
		asm volatile("mov %%cr2, %%eax"
		             : "=a"(err));
		kprintf("Caused by 0x%x\n", err);
	}
	asm volatile("hlt");
}

//Messages
char *exception_messages[] = {
	"EXC0 Division by Zero",
	"EXC1 Debug",
	"EXC2 Non Maskable Interrupt",
	"EXC3 Breakpoint",
	"EXC4 Overflow",
	"EXC5 Bounds",
	"EXC6 Invalid Optcode",
	"EXC7 Coprocessor not available",
	"EXC8 Double fault",
	"EXC9 Coprocessor Segment Overrun",
	"EXC0A Invalid Task State Segment",
	"EXC0B Segment not present",
	"EXC0C Stack Fault",
	"EXC0D General protection fault",
	"EXC0E Page fault",
	"EXC0F reserved",
	"EXC10 Math Fault",
	"EXC11 Alignment Check",
	"EX12 Machine Check"
};
