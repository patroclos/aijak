#include <drivers/ata.h>

#include <device.h>
#include <display/display.h>
#include <memory/memory.h>
#include <pit.h>
#include <system.h>
#include <tasking.h>

MODULE("ATA");

#define ATA_PRIMARY_IO 0x1F0
#define ATA_SECONDARY_IO 0x170

#define ATA_PRIMARY_DCR_AS 0x3F6
#define ATA_SECONDARY_DCR_AS 0x376

#define ATA_PRIMARY_IRQ 14
#define ATA_SECONDARY_IRQ 15

uint8 ata_pm = 0;
uint8 ata_ps = 0;
uint8 ata_sm = 0;
uint8 ata_ss = 0;

uint8 *ide_buf = 0;

void ide_select_drive(uint8 bus, uint8 i)
{
	if (bus == ATA_PRIMARY)
		if (i == ATA_MASTER)
			outportb(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL, 0xA0);
		else
			outportb(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL, 0xB0);
	else if (i == ATA_MASTER)
		outportb(ATA_SECONDARY_IO + ATA_REG_HDDEVSEL, 0xA0);
	else
		outportb(ATA_SECONDARY_IO + ATA_REG_HDDEVSEL, 0xB0);
}

uint8 ide_identify(uint8 bus, uint8 drive)
{
	uint16 io = (bus == ATA_PRIMARY) ? ATA_PRIMARY_IO : ATA_SECONDARY_IO;
	ide_select_drive(bus, drive);

	// ata specs say these values must be zero before sending IDENTIFY
	outportb(io + ATA_REG_SECCOUNT0, 0);
	outportb(io + ATA_REG_LBA0, 0);
	outportb(io + ATA_REG_LBA1, 0);
	outportb(io + ATA_REG_LBA2, 0);

	// send IDENTIFY
	outportb(io + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
	mprint("sent IDENTIFY\n");

	uint8 status = inportb(io + ATA_REG_STATUS);
	if (status)
	{
		// poll until BSY is clear
		while ((inportb(io + ATA_REG_STATUS) & ATA_SR_BSY) != 0)
			;
	pm_start_read:
		if (status & ATA_SR_ERR)
		{
			mprint("%s:%s has ERR set. Disabled.\n", bus == ATA_PRIMARY ? "Primary" : "Secondary", drive == ATA_PRIMARY ? "master" : "slave");
			return 0;
		}
		while (!(status & ATA_SR_DRQ))
			goto pm_start_read;
		mprint("%s:%s is online.\n", bus == ATA_PRIMARY ? "Primary" : "Secondary", drive == ATA_PRIMARY ? "master" : "slave");

		// actually read the data
		set_task(0);
		int i;
		for (i                           = 0; i < 256; i++)
			*(uint16 *)(ide_buf + i * 2) = inportw(io + ATA_REG_DATA);
		set_task(1);
		return 1;
	}
	return 0;
}

void ide_400ns_delay(uint16 io)
{
	int i;
	for (i = 0; i < 4; i++)
		inportb(io + ATA_REG_ALTSTATUS);
}

void ide_poll(uint16 io)
{
	ide_400ns_delay(io);

retry:;
	uint8 status = inportb(io + ATA_REG_STATUS);
	if (status & ATA_SR_BSY)
		goto retry;
retry2:
	status = inportb(io + ATA_REG_STATUS);
	if (status & ATA_SR_ERR)
	{
		panic("ERR ser, device failure!\n");
	}
	if (!(status & ATA_SR_DRQ))
		goto retry2;
	return;
}

uint8 ata_read_one(uint8 *buf, uint32 lba, device_t *dev)
{
	// only supports 28 bit lba so far
	uint8  drive = ((ide_private_data *)(dev->priv))->drive;
	uint16 io    = 0;
	switch (drive)
	{
	case (ATA_PRIMARY << 1 | ATA_MASTER):
		io    = ATA_PRIMARY_IO;
		drive = ATA_MASTER;
		break;
	case (ATA_PRIMARY << 1 | ATA_SLAVE):
		io    = ATA_PRIMARY_IO;
		drive = ATA_SLAVE;
		break;
	case (ATA_SECONDARY << 1 | ATA_MASTER):
		io    = ATA_SECONDARY_IO;
		drive = ATA_MASTER;
		break;
	case (ATA_SECONDARY << 1 | ATA_SLAVE):
		io    = ATA_SECONDARY_IO;
		drive = ATA_SLAVE;
		break;
	default:
		kprintf("FATAL: unknown drive!\n");
		return 0;
	}

	uint8 cmd = (drive == ATA_MASTER ? 0xE0 : 0xf0);
	//uint8 slavebit = (drive == ATA_MASTER ? 0x00 : 0x01);

	outportb(io + ATA_REG_HDDEVSEL, (cmd | (uint8)((lba >> 24 & 0x0f))));
	outportb(io + 1, 0x00);
	outportb(io + ATA_REG_SECCOUNT0, 1);
	outportb(io + ATA_REG_LBA0, (uint8)((lba)));
	outportb(io + ATA_REG_LBA1, (uint8)((lba) >> 8));
	outportb(io + ATA_REG_LBA2, (uint8)((lba) >> 16));
	outportb(io + ATA_REG_COMMAND, ATA_CMD_READ_PIO);

	ide_poll(io);

	set_task(0);
	int i;
	for (i = 0; i < 256; i++)
	{
		uint16 data              = inportw(io + ATA_REG_DATA);
		*(uint16 *)(buf + i * 2) = data;
	}
	ide_400ns_delay(io);
	set_task(1);
	return 1;
}

uint8 ata_write_one(uint8 *buf, uint32 lba, device_t *dev)
{
	// only supports 28 bit lba so far
	uint8  drive = ((ide_private_data *)(dev->priv))->drive;
	uint16 io    = 0;
	switch (drive)
	{
	case (ATA_PRIMARY << 1 | ATA_MASTER):
		io    = ATA_PRIMARY_IO;
		drive = ATA_MASTER;
		break;
	case (ATA_PRIMARY << 1 | ATA_SLAVE):
		io    = ATA_PRIMARY_IO;
		drive = ATA_SLAVE;
		break;
	case (ATA_SECONDARY << 1 | ATA_MASTER):
		io    = ATA_SECONDARY_IO;
		drive = ATA_MASTER;
		break;
	case (ATA_SECONDARY << 1 | ATA_SLAVE):
		io    = ATA_SECONDARY_IO;
		drive = ATA_SLAVE;
		break;
	default:
		kprintf("FATAL: unknown drive!\n");
		return 0;
	}

	uint8 cmd = (drive == ATA_MASTER ? 0xE0 : 0xf0);
	//uint8 slavebit = (drive == ATA_MASTER ? 0x00 : 0x01);

	outportb(io + ATA_REG_HDDEVSEL, (cmd | (uint8)((lba >> 24 & 0x0f))));
	outportb(io + 1, ATA_WRITE);
	outportb(io + ATA_REG_SECCOUNT0, 1);
	outportb(io + ATA_REG_LBA0, (uint8)((lba)));
	outportb(io + ATA_REG_LBA1, (uint8)((lba) >> 8));
	outportb(io + ATA_REG_LBA2, (uint8)((lba) >> 16));
	outportb(io + ATA_REG_COMMAND, ATA_CMD_WRITE_PIO);

	ide_poll(io);

	set_task(0);
	int i;
	for (i = 0; i < 256; i++)
	{
		outportw(io + ATA_REG_DATA, *(uint16 *)(buf + i * 2));
	}
	ide_400ns_delay(io);
	set_task(1);
	return 1;
}

uint8 ata_read(uint8 *buf, uint32 lba, uint32 numsects, device_t *dev)
{
	int i;
	for (i = 0; i < numsects; i++)
	{
		ata_read_one(buf, lba + i, dev);
		buf += 512;
	}
	return 1;
}

uint8 ata_write(uint8 *buf, uint32 lba, uint32 numsects, device_t *dev)
{
	int i;
	for (i = 0; i < numsects; i++)
	{
		ata_write_one(buf, lba + i, dev);
		buf += 512;
	}
	return 1;
}

void ata_probe()
{
	// first check primary bus and master drive
	if (ide_identify(ATA_PRIMARY, ATA_MASTER))
	{
		ata_pm                 = 1;
		device_t *        dev  = (device_t *)malloc(sizeof(device_t));
		ide_private_data *priv = (ide_private_data *)malloc(sizeof(ide_private_data));

		// process IDENTIFY data
		// model goes from W#27 to W#46
		char *str = (char *)malloc(40);
		int   i;
		for (i = 0; i < 40; i += 2)
		{
			str[i]     = ide_buf[ATA_IDENT_MODEL + i + 1];
			str[i + 1] = ide_buf[ATA_IDENT_MODEL + i];
		}
		dev->name      = str;
		dev->unique_id = 32;
		dev->dev_type  = DEVICE_BLOCK;
		priv->drive    = (ATA_PRIMARY << 1) | ATA_MASTER;
		dev->priv      = priv;
		dev->read      = (uint8(*)(uint8 *, uint32, uint32, void *))ata_read;
		dev->write     = (uint8(*)(uint8 *, uint32, uint32, void *))ata_write;
		device_add(dev);
		kprintf("Device: %s\n", dev->name);
	}
	ide_identify(ATA_PRIMARY, ATA_SLAVE);
}

void ata_init()
{
	kprintf("Checking for ATA drives\n");
	ide_buf = (uint8 *)malloc(512);
	ata_probe();
	_kill();
}
