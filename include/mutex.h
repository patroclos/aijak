#pragma once

#include <types.h>

typedef struct{
	uint8 locked;
} mutex;

#define MUTEX_DEFINE(name) static mutex name = {.locked=0};

void mutex_lock(mutex *m);
void mutex_unlock(mutex *m);
