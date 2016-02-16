#include "include/screen.h"
#include "include/keyboard.h"
#include "include/string.h"
#include "include/util.h"
#include "include/isr.h"

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

    //test interrupt
    //__asm__("int %0\n" : : "N"(0));

    print("Welcome to AIJAK (AIJAK is just another kernel)\n");
    //irs_install();
    string lineIn = readString();
    print("\nString: ");
    print(lineIn);
    print("\nLength: ");
    dmpnum(strlen(lineIn));
    print("\n\nHalting CPU");

}
