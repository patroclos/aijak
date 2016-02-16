#pragma once

#include "types.h"

void memory_copy(char *src, char *dst, int len);
void memory_set(uint8 *dest, uint8 val, uint32 len);
void int_to_ascii(int n, char str[]);
