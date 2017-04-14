#include "display/textmode/dispi_textmode.h"

#include "system.h"
#include <memory/memory.h>

static DISPLAY d = { 0 };

static inline uint8 __textmode_make_color(uint8 fg, uint8 bg)
{
	return fg | bg << 4;
}

static inline uint16 __textmode_create_entry(char c, uint8 color)
{
	uint16 _char  = (uint16)c;
	uint16 _color = (uint16)color;
	return _char | _color << 8;
}

static inline void __set_cursor(int row, int col)
{
	unsigned short pos = (row * 80) + col;
	// cursor low port to vga index register
	outportb(0x3d4, 0x0f);
	outportb(0x3d5, (unsigned char)(pos & 0xff));
	// cursor high port to vga index register
	outportb(0x3d4, 0x0e);
	outportb(0x3d5, (unsigned char)((pos >> 8) & 0xff));
}

static inline void __update_cursor(DISPLAY *d)
{
	__set_cursor(d->con.cy, d->con.cx);
}

static inline void __textmode_scrollup()
{
	int y;
	for (y = 0; y < TEXTMODE_HEIGHT; y++)
	{
		memcpy((char *)(VGA_MEMORY + y * TEXTMODE_WIDTH * 2), (char *)(VGA_MEMORY + (y + 1) * TEXTMODE_WIDTH * 2), TEXTMODE_WIDTH * 2);
	}

	uint8 entity = __textmode_make_color(d.con.fgcol, d.con.bgcol);
	memset16((uint16 *)(VGA_MEMORY + TEXTMODE_HEIGHT * TEXTMODE_WIDTH * 2), __textmode_create_entry(' ', entity), TEXTMODE_WIDTH * 2);
}

void __texmode_onRegister()
{
	// no startup required
	return;
}

void __texmode_onSet(uint8 id)
{
	d.clear();
	d.puts("Textmode driver initialized as display ");
	d.putc(id + 48);
	d.putc('\n');
	return;
}

DISPLAY *textmode_init()
{
	d.width      = TEXTMODE_WIDTH;
	d.height     = TEXTMODE_HEIGHT;
	d.con.cx     = 0;
	d.con.cy     = 0;
	d.con.fgcol  = COLOR_WHITE;
	d.con.bgcol  = COLOR_BLACK;
	d.puts       = textmode_puts;
	d.putc       = textmode_putc;
	d.clear      = textmode_clear;
	d.onRegister = __texmode_onRegister;
	d.onSet      = __texmode_onSet;
	__set_cursor(0xff, 0xff); // put cursor off screen
	return &d;
}

void textmode_clear()
{
	uint8 entity = __textmode_make_color(d.con.fgcol, d.con.bgcol);
	int   y, x;
	for (y = 0; y < TEXTMODE_HEIGHT; y++)
		for (x = 0; x < TEXTMODE_WIDTH; x++)
		{
			const long index                = y * 2 * TEXTMODE_WIDTH + x * 2;
			*(uint16 *)(VGA_MEMORY + index) = __textmode_create_entry(' ', entity);
		}
	d.con.cx = 0;
	d.con.cy = 0;
}

void textmode_puts(char *s)
{
	while (*s != 0)
		textmode_putc(*s++);
}

void textmode_putc(char c)
{
	bool incx = true;
	if (!c)
		return;
	if (c == '\t')
		d.con.cx += 4;
	if (c == '\r')
	{
		incx = false;
		d.con.cx--;
		c = ' ';
	}
	if (d.con.cx >= TEXTMODE_WIDTH || c == '\n')
	{
		d.con.cx = 0;
		d.con.cy++;
	}
	if (d.con.cy >= TEXTMODE_HEIGHT - 1)
	{
		__textmode_scrollup();
		d.con.cy--;
	}
	if (c == '\n' || c == '\t')
		return;

	const long index                = d.con.cy * 2 * TEXTMODE_WIDTH + d.con.cx * 2;
	*(uint16 *)(VGA_MEMORY + index) = __textmode_create_entry(c, __textmode_make_color(d.con.fgcol, d.con.bgcol));
	if (incx)
		d.con.cx++;
}
