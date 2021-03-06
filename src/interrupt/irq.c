#include <interrupt/irq.h>
#include <util.h>

uint32 irq_routines[16] = { NULL };

void irq_setroutine(uint8 irq, uint32 routine)
{
	irq_routines[irq] = routine;
}

void irq_clearroutine(uint8 irq)
{
	irq_routines[irq] = NULL;
}

void irq_remap()
{
	//save masks
	unsigned char a1, a2;
	a1 = inportb(PIC_MASTER_DATA);
	a2 = inportb(PIC_SLAVE_DATA);

	outportb(PIC_MASTER_COMMAND, 0x11);
	io_wait();
	outportb(PIC_SLAVE_COMMAND, 0x11);
	io_wait();
	outportb(PIC_MASTER_DATA, 0x20);
	io_wait();
	outportb(PIC_SLAVE_DATA, 0x28);
	io_wait();
	outportb(PIC_MASTER_DATA, 0x04);
	io_wait();
	outportb(PIC_SLAVE_DATA, 0x02);
	io_wait();
	outportb(PIC_MASTER_DATA, 0x01);
	io_wait();
	outportb(PIC_SLAVE_DATA, 0x01);
	io_wait();

	//restore saved masks
	outportb(PIC_MASTER_DATA, a1);
	outportb(PIC_SLAVE_DATA, a2);
}

void irq_gates()
{
	set_idt_gate(32, (uint32)_irq0);
	set_idt_gate(33, (uint32)_irq1);
	set_idt_gate(34, (uint32)_irq2);
	set_idt_gate(35, (uint32)_irq3);
	set_idt_gate(36, (uint32)_irq4);
	set_idt_gate(37, (uint32)_irq5);
	set_idt_gate(38, (uint32)_irq6);
	set_idt_gate(39, (uint32)_irq7);
	set_idt_gate(40, (uint32)_irq8);
	set_idt_gate(41, (uint32)_irq9);
	set_idt_gate(42, (uint32)_irq10);
	set_idt_gate(43, (uint32)_irq11);
	set_idt_gate(44, (uint32)_irq12);
	set_idt_gate(45, (uint32)_irq13);
	set_idt_gate(46, (uint32)_irq14);
	set_idt_gate(47, (uint32)_irq15);
}

void irq_install()
{
	irq_remap();
	irq_gates();
	mask_irqs(0x00);
	io_wait();
	IRQ_RES;
}

void mask_irqs(uint16 mask)
{
	outportb(PIC_MASTER_IMR, (uint8)mask);
	outportb(PIC_SLAVE_IMR, (uint8)(mask >> 8));
}

void send_eoi(uint8 irq_num)
{
	if (irq_num > 7)
	{
		outportb(PIC_SLAVE_COMMAND, EOI);
	}
	outportb(PIC_MASTER_COMMAND, EOI);
}

void irq_handler(struct regs *r)
{
	IRQ_OFF;
	int inum = r->int_no - 32;

	void (*handler)(struct regs * r);
	if (inum < 0 || inum > 16)
	{
		handler = NULL;
	}
	else
	{
		handler = (void (*)(struct regs * r))irq_routines[inum];
	}

	if (handler == NULL)
	{
		send_eoi(inum);
	}
	else
	{
		handler(r);
	}

	IRQ_RES;
}
