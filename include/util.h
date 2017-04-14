#pragma once

#include "string.h"
#include "types.h"

char *uitoa(uint32 n, char *str, uint8 base);
char *itoa(int n, char *str, int base);
int32 atoi(const char *str);
