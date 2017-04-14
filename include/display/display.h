#pragma once

#include <stdlib/stdarg.h>
#include <display/console.h>
#include <pit.h>

#define DISPLAY_MAX_DISPIS 8

typedef struct
{
	uint32  width;
	uint32  height;
	CONSOLE con;

	void (*onRegister)();
	void (*onSet)(uint8 id);
	void (*puts)(char *);
	void (*putc)(char);
	void (*clear)();
} DISPLAY;

#define panic(...) {set_task(0); kprintf("\n***GLORIOUS KERNEL PANIC*** in %s at line %d in function: %s\n", __FILE__, __LINE__, __func__);kprintf(__VA_ARGS__);for(;;);}

#define MODULE(name) static char *__MODULE_NAME = name;
#define mprint(...) __mprintf(__MODULE_NAME, __VA_ARGS__);

#define kerror(...)                     \
	{                                   \
		kprintf("***KERNEL OOPS***: "); \
		kprintf(__VA_ARGS__);           \
	}

uint8 display_register(DISPLAY *d);
uint8 display_set_current(uint8 id);
DISPLAY *display_get_current();
void display_clear();

void __mprintf(char *m, ...);
int kprintf(const char *fmt, ...);
