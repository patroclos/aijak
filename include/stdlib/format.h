#pragma once

#include <stdlib/stdarg.h>
#include <types.h>

int32 vsnprintf(char *str, size_t size, const char *fmt, va_list args);
int32 snprintf(char *str, size_t size, const char *fmt, ...);

int32 vsscanf(const char *str, const char *fmt, va_list args);
int32 sscanf(const char *str, const char *fmt, ...);
