bits    32
section .text
    align   4
    dd      0x1BADB002
    dd      0x00
    dd      - (0x1BADB002+0x00)

global start
extern kmain ;function located in kernel.c
start:
    cli ;clears the interrupts
    call kmain ;send processor to contine execution from the kmain function in kernel.c
    hlt ;halt the cpu

extern irq_handler
extern isr_handler

isr_common_stub:
	pusha
	push ds
	push es
	push fs
	push gs
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov eax, esp
	push eax
	mov eax, isr_handler
	call eax
	pop eax
	pop gs
	pop fs
	pop es
	pop ds
	popa
	add esp, 8
	iret


irq_common_stub:
    pusha
    push ds
    push es
    push fs
    push gs
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov eax, esp
    push eax
    mov eax, irq_handler
    call eax
    pop eax
    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8
    iret

%macro ISR_ENTRY 1
	global _isr%1
	_isr%1:
		cli
		push byte 0
		push byte %1
		jmp isr_common_stub
%endmacro

%macro IRQ_ENTRY 2
    global _irq%1
    _irq%1:
        cli
        push byte 0
        push byte %2
        jmp irq_common_stub
%endmacro

%assign isrnum 0
%rep 32
	ISR_ENTRY isrnum
	%assign isrnum isrnum+1
%endrep

;Loop 16 times and define each IRQ callback
%assign irqnum 0
%rep 16
    IRQ_ENTRY irqnum, irqnum+32
    %assign irqnum irqnum+1
%endrep
