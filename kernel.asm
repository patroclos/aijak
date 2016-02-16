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
