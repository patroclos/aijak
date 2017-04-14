#include <drivers/pci.h>

#include <net/arp.h>
#include <display/display.h>
#include <drivers/intel_e1000.h>
#include <memory/memory.h>
#include <pit.h>
#include <system.h>
#include <tasking.h>

MODULE("PCI");

uint8 pci_read8(uint32 id, uint32 reg)
{
	uint32 addr = 0x80000000 | id | (reg & 0xfc);
	outportl(PCI_CONFIG_ADDR, addr);
	return inportb(PCI_CONFIG_DATA + (reg & 0x03));
}

uint16 pci_read16(uint32 id, uint32 reg)
{
	uint32 addr = 0x80000000 | id | (reg & 0xfc);
	outportl(PCI_CONFIG_ADDR, addr);
	return inportw(PCI_CONFIG_DATA + (reg & 0x02));
}

uint32 pci_read32(uint32 id, uint32 reg)
{
	uint32 addr = 0x80000000 | id | (reg & 0xfc);
	outportl(PCI_CONFIG_ADDR, addr);
	return inportl(PCI_CONFIG_DATA);
}

void pci_write8(uint32 id, uint32 reg, uint8 data)
{
	uint32 address = 0x80000000 | id | (reg & 0xfc);
	outportl(PCI_CONFIG_ADDR, address);
	outportw(PCI_CONFIG_DATA + (reg & 0x03), data);
}

void pci_write16(uint32 id, uint32 reg, uint16 data)
{
	uint32 address = 0x80000000 | id | (reg & 0xfc);
	outportl(PCI_CONFIG_ADDR, address);
	outportw(PCI_CONFIG_DATA + (reg & 0x02), data);
}

void pci_write32(uint32 id, uint32 reg, uint32 data)
{
	uint32 address = 0x80000000 | id | (reg & 0xfc);
	outportl(PCI_CONFIG_ADDR, address);
	outportw(PCI_CONFIG_DATA, data);
}

void pci_read_bar(uint32 id, uint32 index, uint32 *address, uint32 *mask)
{
	uint32 reg = PCI_CONFIG_BAR0 + index * sizeof(uint32);

	// get address
	*address = pci_read32(id, reg);

	// get size of the bar
	pci_write32(id, reg, 0xffffffff);
	*mask = pci_read32(id, reg);

	// restore address
	pci_write32(id, reg, *address);
}

void pci_get_bar(PciBar *bar, uint32 id, uint32 index)
{
	uint32 addressLow;
	uint32 maskLow;
	pci_read_bar(id, index, &addressLow, &maskLow);

	if (addressLow & PCI_BAR_64)
	{
		// 64 bit mmio
		uint32 addressHigh;
		uint32 maskHigh;
		pci_read_bar(id, index + 1, &addressHigh, &maskHigh);

		bar->u.address = (void *)(((uintptr_t)addressHigh << 32) | (addressLow & ~0xf));
		bar->size      = ~(((uint64)maskHigh << 32) | (maskLow & ~0xf)) + 1;
		bar->flags     = addressLow & 0xf;
	}
	else if (addressLow & PCI_BAR_IO)
	{
		// io register
		bar->u.port = (uint16)(addressLow & ~0x3);
		bar->size   = (uint16)(~(maskLow & ~0x3) + 1);
		bar->flags  = addressLow & 0x3;
	}
	else
	{
		// 32-bit mmio
		bar->u.address = (void *)(uintptr_t)(addressLow & ~0xf);
		bar->size      = ~(maskLow & ~0xf) + 1;
		bar->flags     = addressLow & 0xf;
	}
}

uint8 pci_get_interrupt_line(uint32 id)
{
	return pci_read8(id, PCI_CONFIG_INTERRUPT_LINE);
}

void pci_visit(uint16 bus, uint16 dev, uint16 func)
{
	uint32 id = PCI_MAKE_ID(bus, dev, func);

	PciDeviceInfo info;
	info.vendorId = pci_read16(id, PCI_CONFIG_VENDOR_ID);
	if (info.vendorId == 0xffff)
		return;

	info.deviceId  = pci_read16(id, PCI_CONFIG_DEVICE_ID);
	info.progIntf  = pci_read8(id, PCI_CONFIG_PROG_INTF);
	info.subclass  = pci_read8(id, PCI_CONFIG_SUBCLASS);
	info.classCode = pci_read8(id, PCI_CONFIG_CLASS_CODE);

	// TODO pciclassname
	mprint("%x:%x:%d 0x%x/0x%x\n", bus, dev, func, info.vendorId, info.deviceId);
	if (info.vendorId == 0x8086 && info.deviceId == 0x100e)
	{
		eth_intel_init(id,&info);
		arp_init();
		int x;
		for (x = 0; x < 1; x++)
		{
			// my mac: 52-54-0-12-34-56
			// my ip: 192.168.2.123
			//arp who is 192.168.2.1 tell 192.168.2.124(52-54-0-12-34-56)
			const char *pkgbuf = "\xff\xff\xff\xff\xff\xff\x52\x54\x00\x12\x34\x56\x08\x06\x00\x01\x08\x00\x06\x04\x00\x01\x52\x54\x00\x12\x34\x56\xc0\xa8\x02\x7b\x00\x00\x00\x00\x00\x00\xc0\xa8\x02\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
			NetBuf *    b      = net_alloc_buf();
			memcpy(b->start, pkgbuf, 58);
			b->end += 58;
			eth_intel_send(b);
		}
	}
}

void pci_init()
{
	mprint("Initializing PCI\n");

	uint32 bus;
	uint32 dev;
	uint32 func;

	for (bus = 0; bus < 256; bus++)
	{
		for (dev = 0; dev < 32; dev++)
		{
			uint32 baseId     = PCI_MAKE_ID(bus, dev, 0);
			uint8  headerType = pci_read8(baseId, PCI_CONFIG_HEADER_TYPE);
			uint32 funcCount  = headerType & PCI_TYPE_MULTIFUNC ? 8 : 1;

			for (func = 0; func < funcCount; func++)
				pci_visit(bus, dev, func);
		}
	}
	_kill();
}
