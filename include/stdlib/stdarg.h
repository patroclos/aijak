#pragma once

typedef void *va_list;

#define __va_size(type) \
	((sizeof(type) + 3) & ~0x3)

#define va_start(va_l, last) \
	((va_l) = (void *)&(last) + __va_size(last))

#define va_end(va_l)

#define va_arg(va_l, type)      \
	((va_l) += __va_size(type), \
	    *((type *)((va_l)-__va_size(type))))
