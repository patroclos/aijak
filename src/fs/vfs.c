#include <fs/vfs.h>

#include <device.h>
#include <display/display.h>
#include <memory/memory.h>
#include <mutex.h>
#include <string.h>
#include <string.h>
#include <tasking.h>
#include <tasking.h>

MODULE("VFS");

#define MAX_MOUNTS 16

uint8 __init_vfs = 0;

mount_info_t **mount_points  = 0;
int            last_mount_id = 0;

device_t *check_mount(char *loc)
{
	int i;
	for (i = 0; i < last_mount_id; i++)
		if (!strcmp(loc, mount_points[i]->loc))
			return mount_points[i]->dev;
	return 0;
}

uint8 list_mount()
{
	int i;
	for (i = 0; i < MAX_MOUNTS; i++)
	{
		if (!mount_points[i])
			break;
		kprintf("%s on %s type: %s\n", mount_points[i]->dev->name, mount_points[i]->loc, mount_points[i]->dev->fs->name);
	}
	return 1;
}

uint8 device_try_to_mount(device_t *dev, char *loc)
{
	if (!dev || !(dev->unique_id))
		return 0;
	if (check_mount(loc))
		return 0;
	//probe
	return 0;
}

static inline uint8 __find_mount(char *filename, int *adjust)
{
	char *orig = (char *)malloc(strlen(filename) + 1);
	memset(orig, 0, strlen(filename) + 1);
	memcpy(orig, filename, strlen(filename) + 1);
	if (orig[strlen(orig)] == '/')
		str_backspace(orig, '/');
	while (1)
	{
		int i;
		for (i = 0; i < MAX_MOUNTS; i++)
		{
			if (!mount_points[i])
				break;
			if (!strcmp(mount_points[i]->loc, orig))
			{
				// adjust the orig to make it relative to fs/dev
				*adjust = (strlen(orig) - 1);
				free(orig);
				return i;
			}
		}
		if (!strcmp(orig, "/"))
			break;
		str_backspace(orig, '/');
	}
	return 0;
}

uint8 vfs_read(char *filename, char *buffer)
{
	// correct algo to resolve mounts:
	// in a loop remove until '/' and then look for match
	// if no match, continue until last '/' and then we know
	// it is on the root_device
	int adjust = 0;
	int i      = __find_mount(filename, &adjust);
	filename += adjust;
	return mount_points[i]->dev->fs->read(filename, buffer, mount_points[i]->dev, mount_points[i]->dev->fs->priv_data);
}

uint32 vfs_ls(char *dir, char *buffer)
{
	// algo:
	// for each mount, backspace one, cmp with 'dir'
	// if yes, print out its dir name
	char *orig = (char *)malloc(strlen(dir) + 1);
	memset(orig, 0, strlen(dir) + 1);
	memcpy(orig, dir, strlen(dir) + 1);
	while (1)
	{
		int i;
		for (i = 0; i < MAX_MOUNTS; i++)
		{
			if (!mount_points[i])
				break;
			// backspace one, check if eq dir, if so print dir name
			// if the mounts location eq the backspaced loc
			if (!strcmp(mount_points[i]->loc, orig))
			{
				// adjust and send
				mount_points[i]->dev->fs->read_dir(dir + strlen(mount_points[i]->loc) - 1, buffer, mount_points[i]->dev, mount_points[i]->dev->fs->priv_data);
				// now we found who hosts this dir, look
				// for those that are mounted to this dirs host
				int k;
				for (k = 0; k < MAX_MOUNTS; k++)
				{
					if (!mount_points[k])
						break;
					char *mount = (char *)malloc(strlen(mount_points[k]->loc) + 1);
					memcpy(mount, mount_points[k]->loc, strlen(mount_points[k]->loc) + 1);
					str_backspace(mount, '/');
					if (!strcmp(mount, dir))
					{
						char *p = mount_points[k]->loc + strlen(dir);
						if (strlen(p) == 0 || strlen(p) == 1)
							continue;
						kprintf("%s\n", p);
					}
					free(mount);
				}
				break;
			}
		}
		if (!strcmp(orig, "/"))
			break;
		str_backspace(orig, '/');
	}
	free(orig);
	return 1;
}

uint8 vfs_exist_in_dir(char *wd, char *fn)
{
	char *filename = (char *)malloc(strlen(wd) + 2 + strlen(fn));
	memset(filename, 0, strlen(wd) + 2 + strlen(fn));
	memcpy(filename, wd, strlen(wd));
	memcpy(filename + strlen(wd), fn, strlen(fn));
	memset(filename + strlen(wd) + strlen(fn) + 1, '\0', 1);

	// algo:
	// for each mount, check if it is mounted to wd
	// if it is, return one

	if (filename[strlen(filename)] != '/')
	{
		uint32 index        = strlen(filename);
		filename[index]     = '/';
		filename[index + 1] = 0;
	}
	int   rc = 0;
	char *o  = (char *)malloc(strlen(filename) + 2);
	memset(o, 0, strlen(filename) + 2);
	memcpy(o, filename, strlen(filename) + 1);

	while (1)
	{
		int i;
		for (i = 0; i < MAX_MOUNTS; i++)
		{
			if (!mount_points[i])
				break;
			if (!strcmp(o, mount_points[i]->loc))
			{
				filename += strlen(mount_points[i]->loc) - 1;
				rc = mount_points[i]->dev->fs->exist(filename, mount_points[i]->dev, mount_points[i]->dev->fs->priv_data);
				free(o);
				free(filename);
				return rc;
			}
		}
		if (!strcmp(o, "/"))
			break;
		str_backspace(o, '/');
	}
	free(o);
	free(filename);
	return rc;
}

void vfs_init()
{
	mprint("Loading VFS\n");
	mount_points = (mount_info_t **)malloc(sizeof(uint32) * MAX_MOUNTS);
	__init_vfs   = 1;
	_kill();
}
