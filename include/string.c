#include "string.h"

uint16 strlen(string ch)
{
    uint16 i = 0;
    while(ch[i++]);
    return --i;
}

uint8 strEqual(string ch1, string ch2)
{
    uint8 size = strlen(ch1);
    if(size != strlen(ch2))
    {
            return 0;
    }
    else
    {
        uint8 i = 0;
        for(;i<=size;i++)
        {
            if(ch1[i] != ch2[i]) return 0;
        }
    }
    return 1;
}

void strReverse(string s)
{
    int i, j;
    uint8 c;

    for(i=0, j=strlen(s)-1; i<j; i++, j--)
    {
        c=s[i];
        s[i]=s[j];
        s[j]=c;
    }
}
