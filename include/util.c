#include "util.h"

void memory_copy(char *source, char *dest, int len)
{
    int i;
    for(i = 0; i < len; i++)
    {
        *(dest + i) = *(source + i); //dest[i]=source[i];
    }
}

void memory_set(uint8 *dest, uint8 val, uint32 len)
{
    uint8 *temp = (uint8 *)dest;
    for(; len!=0; len--) *temp++ = val;
}

/*
K&R implementation
*/
void int_to_ascii(int n, string str)
{
    int i, sign;
    if((sign = n) < 0) n = -n;
    i = 0;
    do {
        str[i++] = n % 10 + '0';
    } while((n /= 10) > 0);

    if(sign < 0) str[i++] = '-';
    str[i] = '\0';
    strReverse(str);
}
