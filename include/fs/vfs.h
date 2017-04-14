#pragma once

#include <device.h>

struct __device_t;

typedef struct __fs_t
{
	char *name;
	uint8 (*probe)(struct __device_t *);
	uint8 (*read)(char *, char *, struct __device_t *, void *);
	uint8 (*read_dir)(char *, char *, struct __device_t *, void *);
	uint8 (*touch)(char *fn, struct __device_t *, void *);
	uint8 (*writefile)(char *fn, char *buf, uint32 len, struct __device_t *, void *);
	uint8 (*exist)(char *filename, struct __device_t *, void *);
	uint8 (*mount)(struct __device_t *, void *);
	uint8 *priv_data;
} filesystem_t;

typedef struct __mount_info_t
{
	char *             loc;
	struct __device_t *dev;
} mount_info_t;

uint8 vfs_read(char *f, char *buf);
uint32 vfs_ls(char *d, char *buf);
uint8 vfs_exist_in_dir(char *wd, char *fn);

void vfs_init();

uint8 list_mount();

uint8 device_try_to_mount(struct __device_t *dev, char *location);
