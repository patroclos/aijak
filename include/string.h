#pragma once

#include "types.h"

size_t strlen(const char *ch);

uint8 strcmp(const char *s1, const char *s2);

uint8 strncmp(const char *s1, const char *s2, size_t num);

char **strtok(char *str, char *delim, size_t *length);

bool strprefix(const char *prefix, const char *str);

uint8 str_backspace(char *s, char c);

void strReverse(char *s);
