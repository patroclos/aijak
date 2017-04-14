#pragma once

#include <types.h>

void keyboard_init();

uint8 keyboard_enabled();
char keyboard_get_key();
uint8 keyboard_to_ascii(uint8 code);

