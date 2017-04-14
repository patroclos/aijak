#include <drivers/keyboard.h>

#include <display/display.h>
#include <interrupt/irq.h>
#include <memory/memory.h>
#include <mutex.h>
#include <string.h>
#include <system.h>
#include <tasking.h>

MODULE("KBD");

void keyboard_irq(struct regs *r);

uint8  lastkey       = 0;
uint8 *keycache      = 0;
uint16 key_loc       = 0;
uint8  __kbd_enabled = 0;

enum KEYCODE
{
	NULL_KEY   = 0,
	Q_PRESSED  = 0x10,
	Q_RELEASED = 0x90,
	W_PRESSED  = 0x11,
	W_RELEASED = 0x91,
	E_PRESSED  = 0x12,
	E_RELEASED = 0x92,
	R_PRESSED  = 0x13,
	R_RELEASED = 0x93,
	T_PRESSED  = 0x14,
	T_RELEASED = 0x94,
	Z_PRESSED  = 0x15,
	Z_RELEASED = 0x95,
	U_PRESSED  = 0x16,
	U_RELEASED = 0x96,
	I_PRESSED  = 0x17,
	I_RELEASED = 0x97,
	O_PRESSED  = 0x18,
	O_RELEASED = 0x98,
	P_PRESSED  = 0x19,
	P_RELEASED = 0x99,
	A_PRESSED  = 0x1E,
	A_RELEASED = 0x9E,
	S_PRESSED  = 0x1F,
	S_RELEASED = 0x9F,
	D_PRESSED  = 0x20,
	D_RELEASED = 0xA0,
	F_PRESSED  = 0x21,
	F_RELEASED = 0xA1,
	G_PRESSED  = 0x22,
	G_RELEASED = 0xA2,
	H_PRESSED  = 0x23,
	H_RELEASED = 0xA3,
	J_PRESSED  = 0x24,
	J_RELEASED = 0xA4,
	K_PRESSED  = 0x25,
	K_RELEASED = 0xA5,
	L_PRESSED  = 0x26,
	L_RELEASED = 0xA6,
	Y_PRESSED  = 0x2C,
	Y_RELEASED = 0xAC,
	X_PRESSED  = 0x2D,
	X_RELEASED = 0xAD,
	C_PRESSED  = 0x2E,
	C_RELEASED = 0xAE,
	V_PRESSED  = 0x2F,
	V_RELEASED = 0xAF,
	B_PRESSED  = 0x30,
	B_RELEASED = 0xB0,
	N_PRESSED  = 0x31,
	N_RELEASED = 0xB1,
	M_PRESSED  = 0x32,
	M_RELEASED = 0xB2,

	ZERO_PRESSED = 0xB,
	ONE_PRESSED  = 0x2,

	POINT_PRESSED  = 0x34,
	POINT_RELEASED = 0xB4,

	SLASH_RELEASED = 0xB5,

	TAB_PRESSED        = 0xF,
	BACKSPACE_PRESSED  = 0xE,
	BACKSPACE_RELEASED = 0x8E,
	SPACE_PRESSED      = 0x39,
	SPACE_RELEASED     = 0xB9,
	ENTER_PRESSED      = 0x1C,
	ENTER_RELEASED     = 0x9C,

};

#define KBD_SEND(byt) outportb(0x64, byt);

void keyboard_init()
{
	mprint("PS/2 Keyboard initializing.\n");
	keycache = (uint8 *)malloc(256);
	memset(keycache, 0, 256);

	irq_setroutine(1, (uint32)keyboard_irq);
	__kbd_enabled = 1;
	_kill();
}

uint8 keyboard_enabled()
{
	return __kbd_enabled;
}

void keyboard_read_key()
{
	lastkey = 0;
	if (inportb(0x64) & 1)
		lastkey = inportb(0x60);
}

MUTEX_DEFINE(m_getkey);
static char ch = 0;

char keyboard_get_key()
{
	mutex_lock(&m_getkey);
	ch = 0;
	if (key_loc == 0)
		goto out;
	ch = *keycache;
	key_loc--;
	int i;
	for (i          = 0; i < 256; i++)
		keycache[i] = keycache[i + 1];
out:
	mutex_unlock(&m_getkey);
	return ch;
}

static char *numrow = "1234567890";
static char *krow1  = "qwertzuiop";
static char *krow2  = "asdfghjkl";
static char *krow3  = "yxcvbnm";

uint8 keyboard_to_ascii(uint8 scancode)
{
	uint8 c = 0;

	if (scancode == ENTER_PRESSED)
		return '\n';
	if (scancode == SPACE_PRESSED)
		return ' ';
	if (scancode == BACKSPACE_PRESSED)
		return '\r';
	if (scancode == TAB_PRESSED)
		return '\t';
	if (scancode == POINT_PRESSED)
		return '.';
	if (scancode == SLASH_RELEASED)
		return '/';
	if (scancode == ZERO_PRESSED)
		return '0';

	if (scancode >= ONE_PRESSED && scancode <= ZERO_PRESSED)
		c = numrow[scancode - ONE_PRESSED];
	else if (scancode >= Q_PRESSED && scancode <= P_PRESSED)
		c = krow1[scancode - Q_PRESSED];
	else if (scancode >= A_PRESSED && scancode <= L_PRESSED)
		c = krow2[scancode - A_PRESSED];
	else if (scancode >= Y_PRESSED && scancode <= M_PRESSED)
		c = krow3[scancode - Y_PRESSED];

	return c;
}

void keyboard_irq(struct regs *r)
{
	keycache[key_loc++] = keyboard_to_ascii(inportb(0x60));
	send_eoi(1);
}
