#include <stdlib/format.h>

#include <display/display.h>
#include <memory/memory.h>
#include <string.h>

// TODO implement 64 bit arithmatic

enum
{
	PAD_ZERO = 1,
	PAD_LEFT = 2,
};

typedef struct __formatter_t
{
	char * p;
	char * end;
	uint32 flags;
	int32  width;
} formatter_t;

bool _is_space(char c)
{
	return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\f' || c == '\v';
}

bool _is_digit(char c)
{
	return c >= '0' && c <= '9';
}

void _outc(formatter_t *f, char c)
{
	if (f->p < f->end)
		*f->p++ = c;
}

void _outs(formatter_t *f, const char *s)
{
	int  width   = f->width;
	char padchar = f->flags & PAD_ZERO ? '0' : ' ';

	if (~f->flags & PAD_LEFT)
		while (--width >= 0)
			_outc(f, padchar);

	while (*s)
		_outc(f, *s++);

	while (--width >= 0)
		_outc(f, padchar);
}

void _outdec(formatter_t *f, uint32 n)
{
	char  buf[32] = { 0 };
	char *end     = buf + sizeof(buf) - 1;
	char *s       = end;
	*s            = 0;

	do
	{
		char c = '0' + (n % 10);
		*--s   = c;
		n /= 10;
	} while (n > 0);

	f->width -= end - s;
	_outs(f, s);
}

void _outhex(formatter_t *f, char type, uint32 n)
{
	char  buf[32] = { 0 };
	char *end     = buf + sizeof(buf) - 1;
	char *s       = end;
	*s            = 0;

	do
	{
		uint8 digit = n & 0xf;
		char  c;
		if (digit < 10)
			c = '0' + digit;
		else if (type == 'x')
			c = 'a' + digit - 10;
		else
			c = 'A' + digit - 10;
		*--s  = c;
		n >>= 4;
	} while (n > 0);
	f->width -= end - s;
	_outs(f, s);
}

void _outpointer(formatter_t *f, void *p)
{
	uint32 n = (uintptr_t)p;
	_outhex(f, 'x', n);
}

int32 vsnprintf(char *str, size_t size, const char *fmt, va_list args)
{
	formatter_t f;
	f.p   = str;
	f.end = str + size - 1;

	for (;;)
	{
		char c = *fmt++;
		if (!c)
			break;

		if (c != '%')
		{
			_outc(&f, c);
			continue;
		}

		c = *fmt++;

		f.flags = 0;
		if (c == '-')
		{
			f.flags |= PAD_LEFT;
			c = *fmt++;
		}
		else if (c == '0')
		{
			f.flags |= PAD_ZERO;
			c = *fmt++;
		}

		// parse width
		f.width = -1;
		if (_is_digit(c))
		{
			int width = 0;
			do
			{
				width = width * 10 + c - '0';
				c     = *fmt++;
			} while (_is_digit(c));
			f.width = width;
		}

		// process type specifier

		char type = c;
		switch (type)
		{
		case '%':
			_outc(&f, '%');
			break;
		case 'c':
			c = va_arg(args, int);
			_outc(&f, c);
			break;
		case 's':
		{
			char *s = va_arg(args, char *);
			if (!s)
				s = "(null)";

			if (f.width > 0)
			{
				char *p = s;
				while (*p)
					++p;
				f.width -= p - s;
			}
			_outs(&f, s);
		}
		break;
		case 'd':
		{
			int32 n = va_arg(args, int32);

			if (n < 0)
			{
				_outc(&f, '-');
				n = -n;
			}
			_outdec(&f, n);
		}
		break;
		case 'u':
		{
			uint32 n = va_arg(args, uint32);

			_outdec(&f, n);
		}
		break;
		case 'x':
		case 'X':
		{
			uint32 n = va_arg(args, uint32);

			_outhex(&f, type, n);
		}
		break;
		case 'p':
		{
			void *p = va_arg(args, void *);
			_outc(&f, '0');
			_outc(&f, 'x');
			_outpointer(&f, p);
		}
		break;
		}
	}
	if (f.p < f.end + 1)
		*f.p = 0;
	return f.p - str;
}

int32 snprintf(char *str, size_t size, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int32 len = vsnprintf(str, size, fmt, args);
	va_end(args);
	return len;
}

int32 vsscanf(const char *str, const char *fmt, va_list args)
{
	int32 count = 0;

	for (;;)
	{
		char c = *fmt++;
		if (!c)
			break;
		if (_is_space(c))
			while (_is_space(*str))
				++str;
		else if (c != '%')
		{
		match_literal:
			// non-format char
			if (*str == 0)
				goto end_of_input;

			if (*str != c)
				goto match_failure;
			++str;
		}
		else
		{
			c = *fmt++;

			char type = c;
			switch (type)
			{
			case '%':
				goto match_literal;
			case 'd':
			{
				int32 sign = 1;
				c          = *str++;
				if (c == 0)
					goto end_of_input;

				if (c == '-')
				{
					sign = -1;
					c    = *str++;
				}

				int32 n = 0;
				while (_is_digit(c))
				{
					n = n * 10 + c - '0';
					c = *str++;
				}
				n *= sign;
				--str;

				int32 *result = va_arg(args, int32 *);
				*result       = n;
				count++;
			}
			break;
			}
		}
	}
match_failure:
	return count;
end_of_input:
	return count ? count : -1;
}

int32 sscanf(const char *str, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int32 count = vsscanf(str, fmt, args);
	va_end(args);
	return count;
}
