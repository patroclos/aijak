#pragma once

#include <types.h>

// TODO vfs

struct __fs_t;

typedef enum __device_type {
	DEVICE_UNKNOWN = 0,
	DEVICE_CHAR    = 1,
	DEVICE_BLOCK   = 2,
} device_type;

typedef struct __device_t
{
	char *         name;
	uint32         unique_id;
	device_type    dev_type;
	struct __fs_t *fs;
	uint8 (*read)(uint8 *buffer, uint32 offset, uint32 len, void *dev);
	uint8 (*write)(uint8 *buffer, uint32 offset, uint32 len, void *dev);
	void *priv;
} device_t;

void device_print_out();

void device_init();
int device_add(device_t *dev);
device_t *device_get(uint32 id);
device_t *device_get_by_id(uint32 id);
int device_getnumber();
