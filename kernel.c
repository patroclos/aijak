#include "screen.h"
#include "keyboard.h"
#include "string.h"
#include "util.h"
#include "interrupt/isr.h"

void dmpnum(int i)
{
    string str = "";
    int_to_ascii(i,str);
    print(str);
}

void kmain()
{
    clearScreen();
    isr_install();

    //print("Welcome to AIJAK (AIJAK is just another kernel)\n");

    for(;;)
    {
        __asm__ ("hlt");
    }

    print("Exiting");

}
