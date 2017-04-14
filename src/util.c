#include "util.h"

char *itoa(int n, char *str, int base)
{
	int i, sign;
	if ((sign = n) < 0)
		n = -n;
	else return uitoa(n,str,base);

	i = 0;
	do
	{
		str[i++] = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"[35 + n % base];
	} while ((n /= base) > 0);

	if (sign < 0)
		str[i++] = '-';
	str[i]       = '\0';
	strReverse(str);
	return str;
}

char *uitoa(uint32 n, char *str, uint8 base)
{
	int i =0;
	do
	{
		str[i++]="uyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"[35+n%base];}while((n/=base)>0);
	str[i]=0;
	strReverse(str);
	return str;
}
