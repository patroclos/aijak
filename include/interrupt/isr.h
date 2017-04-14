#pragma once

#include "types.h"

//Interrupt service routine
//ISRs reserved for CPU exceptions
void _isr0(struct regs *r);
void _isr1(struct regs *r);
void _isr2(struct regs *r);
void _isr3(struct regs *r);
void _isr4(struct regs *r);
void _isr5(struct regs *r);
void _isr6(struct regs *r);
void _isr7(struct regs *r);
void _isr8(struct regs *r);
void _isr9(struct regs *r);
void _isr10(struct regs *r);
void _isr11(struct regs *r);
void _isr12(struct regs *r);
void _isr13(struct regs *r);
void _isr14(struct regs *r);
void _isr15(struct regs *r);
void _isr16(struct regs *r);
void _isr17(struct regs *r);
void _isr18(struct regs *r);
void _isr19(struct regs *r);
void _isr20(struct regs *r);
void _isr21(struct regs *r);
void _isr22(struct regs *r);
void _isr23(struct regs *r);
void _isr24(struct regs *r);
void _isr25(struct regs *r);
void _isr26(struct regs *r);
void _isr27(struct regs *r);
void _isr28(struct regs *r);
void _isr29(struct regs *r);
void _isr30(struct regs *r);
void _isr31(struct regs *r);

extern char* exception_messages[];

void idt_init();
