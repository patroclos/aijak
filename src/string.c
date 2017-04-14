#include "string.h"

#include <memory/memory.h>

size_t strlen(const char *ch)
{
	size_t i = 0;
	while (ch[i++])
		;
	return --i;
}

uint8 strcmp(const char *s1, const char *s2)
{
	for (; *s1 == *s2; s1++, s2++)
		if (*s1 == 0)
			return 0;
	return ((*(unsigned char *)s1 < *(unsigned char *)s2) ? -1 : 1);
}

uint8 strncmp(const char *s1, const char *s2, size_t num)
{
	for (; *s1 == *s2; s1++, s2++)
		if (*s1 == 0 || !num--)
			return 0;
	return ((*(unsigned char *)s1 < *(unsigned char *)s2) ? -1 : 1);
}

char **strtok(char *str, char *delim, size_t *length)
{
	size_t numS = strlen(str);
	size_t numD = strlen(delim);

	size_t is;
	size_t id;
	*length = 1;

	for (is = 0; is < numS; is++)
		for (id = 0; id < numD; id++)
			if (str[is] == delim[id])
			{
				*length = *length + 1;
				break;
			}

	char **tokens = (char **)malloc(*length * sizeof(char *));
	tokens[0]     = str;

	size_t n = 1;
	for (is = 0; is < numS; is++)
		for (id = 0; id < numD; id++)
			if (str[is] == delim[id])
			{
				str[is]     = 0;
				tokens[n++] = str + is + 1;
				break;
			}

	return tokens;
}

bool strprefix(const char *prefix, const char *str)
{
	return strncmp(prefix, str, strlen(prefix)) == 0;
}

uint8 str_backspace(char *s, char c)
{
	size_t i = strlen(s);
	while (--i)
	{
		if (s[i] == c)
		{
			s[i + 1] = 0;
			return 1;
		}
	}
	return 0;
}

void strReverse(char *s)
{
	int   i, j;
	uint8 c;

	for (i = 0, j = strlen(s) - 1; i < j; i++, j--)
	{
		c    = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}
