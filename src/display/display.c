#include <display/display.h>

#include <mutex.h>
#include <string.h>
#include <util.h>

// variables

static DISPLAY *dispis[DISPLAY_MAX_DISPIS];
static uint8    _last_register = 1;
static uint8    current        = 0;

static DISPLAY *cd = 0;

int __kprint_va_list(char *fmt, va_list ap);

MUTEX_DEFINE(m_display);

void __mprintf(char *m, ...)
{
	va_list ap;
	va_start(ap, m);
	kprintf("[%s]: ", m);
	char *fmt = va_arg(ap, char *);
	__kprint_va_list(fmt, ap);
}

int kprintf(const char *fmt, ...)
{
	if (!fmt)
		return 0;
	va_list ap;
	va_start(ap, fmt);
	return __kprint_va_list((char *)fmt, ap);
}

int __kprint_va_list(char *fmt, va_list ap)
{
	mutex_lock(&m_display);
	char *       s   = 0;
	unsigned int len = 0;

	uint16 i;
	for (i = 0; i < strlen(fmt); i++)
	{
		if (fmt[i] != '%')
		{
			cd->putc(fmt[i]);
			len++;
			continue;
		}

		switch (fmt[++i])
		{
		case 's':
		{
			s = va_arg(ap, char *);
			cd->puts(s);
			len += strlen(s);
			continue;
		}
		case 'd':
		{
			int  c       = va_arg(ap, int);
			char str[16] = { 0 };
			itoa(c, str, 10);
			cd->puts(str);
			len += strlen(str);
			continue;
		}
		case 'x':
		{
			int  c       = va_arg(ap, int);
			char str[32] = { 0 };
			uitoa(c, str, 16);
			cd->puts(str);
			len += strlen(str);
			continue;
		}
		case 'b': //binary
		{
			int  c       = va_arg(ap, int);
			char str[32] = { 0 };
			itoa(c, str, 2);
			cd->puts(str);
			len += strlen(str);
			continue;
		}
		case 'c':
		{
			char c = (char)(va_arg(ap, int) & ~0xffffff00);
			cd->putc(c);
			len++;
			continue;
		}
		default:
			i--;
		}
	}
	mutex_unlock(&m_display);
	va_end(ap);
	return len;
}

// interface methods

uint8 display_register(DISPLAY *d)
{
	dispis[_last_register] = d;
	dispis[_last_register]->onRegister();
	return _last_register++;
}

uint8 display_set_current(uint8 id)
{
	if (current == id)
		return 0;
	current = id;
	dispis[current]->onSet(id);
	cd = dispis[current];
	return 1;
}

DISPLAY *display_get_current()
{
	return cd;
}

void display_clear()
{
	cd->clear();
}
