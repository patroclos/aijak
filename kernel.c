#include <system.h>
#include <screen.h>
#include <keyboard.h>
#include <string.h>
#include <util.h>
#include <interrupt/isr.h>
#include <interrupt/irq.h>

void dmpnum(int i)
{
    string str = {NULL};
    int_to_ascii(i,str);
    print(str);
}

void kbhandle(struct regs *r)
{
    print("KB\n");
    while(inportb(0x64) & 2);
    dmpnum(inportb(0x60));
    send_eoi(1);
}

uint32 t = 0;

void tickhandle(struct regs *r)
{
    //dmpnum(t++ % 20);
    if(t++ % 20 == false)
    {
        dmpnum(t / 20);
        print("\n");
    }
    send_eoi(0);
}

void kmain()
{
    clearScreen();

    isr_install();
    irq_install();

    irq_setroutine(0, (uint32)tickhandle);
    irq_setroutine(1, (uint32)kbhandle);

    print("Welcome to AIJAK (AIJAK is just another kernel)\n");

    for(;;)
    {
        asm ("hlt");
    }
}
