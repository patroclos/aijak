#include "keyboard.h"

string readString()
{
    string buffstr = "";
    uint8 i = 0;
    uint8 reading = 1;
    while (reading)
    {
        if(inportb(0x64) & 0x1)
        {
            switch (inportb(0x60))
            {
                /* ESC Button
                case 1:
                    printch((char)27);
                    buffstr[i]=(char)27;
                    i++;
                    break;*/
                case 2:
                    printch('1');
                    buffstr[i] = '1';
                    i++;
                    break;
                case 3:
                    printch('2');
                    buffstr[i] = '2';
                    i++;
                    break;
                case 4:
                    printch('3');
                    buffstr[i] = '3';
                    i++;
                    break;
                case 0x1C:
                    reading=0;
                    break;
                default:
                    break;
            }
        }
    }
    buffstr[i] = '\0';
    return buffstr;
}
