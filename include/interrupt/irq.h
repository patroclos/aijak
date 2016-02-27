#include "system.h"
#include "idt.h"

#define PIC_MASTER_COMMAND	0x20
#define PIC_MASTER_DATA		0x21
#define PIC_MASTER_IMR		0x21
#define PIC_SLAVE_COMMAND	0xA0
#define PIC_SLAVE_DATA		0xA1
#define PIC_SLAVE_IMR		0xA1

#define EOI	0x20

//IRQ INSTRUCTIONS
#define IRQ_OFF { asm volatile ("cli"); }
#define IRQ_RES { asm volatile ("sti"); }

uint32 irq_routines[16];

void irq_setroutine(uint8 irq, uint32 routine);
void irq_clearroutine(uint8 irq);

void _irq0();
void _irq1();
void _irq2();
void _irq3();
void _irq4();
void _irq5();
void _irq6();
void _irq7();
void _irq8();
void _irq9();
void _irq10();
void _irq11();
void _irq12();
void _irq13();
void _irq14();
void _irq15();

void irq_gates();

void irq_remap();

void mask_irqs(uint16 mask);

void send_eoi(uint8 irq_num);

void irq_install();

void irq_handler(struct regs *r);
