#pragma once

#include <types.h>

#define PCI_MAKE_ID(bus, dev, func) ((bus) << 16) | ((dev) << 11) | ((func) << 8)

// IO ports
#define PCI_CONFIG_ADDR 0xCF8
#define PCI_CONFIG_DATA 0xCFC

// Header Type
#define PCI_TYPE_MULTIFUNC 0x80
#define PCI_TYPE_GENERIC 0x00
#define PCI_TYPE_PCI_BRIDGE 0x01
#define PCI_TYPE_CARDBUS_BRIDGE 0x02

// PCI configuration registers
#define PCI_CONFIG_VENDOR_ID 0x00
#define PCI_CONFIG_DEVICE_ID 0x02
#define PCI_CONFIG_COMMAND 0x04
#define PCI_CONFIG_STATUS 0x06
#define PCI_CONFIG_REVISION_ID 0x08
#define PCI_CONFIG_PROG_INTF 0x09
#define PCI_CONFIG_SUBCLASS 0x0a
#define PCI_CONFIG_CLASS_CODE 0x0b
#define PCI_CONFIG_CACHELINE_SIZE 0x0c
#define PCI_CONFIG_LATENCY 0x0d
#define PCI_CONFIG_HEADER_TYPE 0x0e
#define PCI_CONFIG_BIST 0x0f

// Type 0x00 (Generic) Configuration Registers
#define PCI_CONFIG_BAR0 0x10
#define PCI_CONFIG_BAR1 0x14
#define PCI_CONFIG_BAR2 0x18
#define PCI_CONFIG_BAR3 0x1c
#define PCI_CONFIG_BAR4 0x20
#define PCI_CONFIG_BAR5 0x24
#define PCI_CONFIG_CARDBUS_CIS 0x28
#define PCI_CONFIG_SUBSYSTEM_VENDOR_ID 0x2c
#define PCI_CONFIG_SUBSYSTEM_DEVICE_ID 0x2e
#define PCI_CONFIG_EXPANSION_ROM 0x30
#define PCI_CONFIG_CAPABILITIES 0x34
#define PCI_CONFIG_INTERRUPT_LINE 0x3c
#define PCI_CONFIG_INTERRUPT_PIN 0x3d
#define PCI_CONFIG_MIN_GRANT 0x3e
#define PCI_CONFIG_MAX_LATENCY 0x3f

// PCI BAR

#define PCI_BAR_IO 0x01
#define PCI_BAR_LOWMEM 0x02
#define PCI_BAR_64 0x04
#define PCI_BAR_PREFETCH 0x08

typedef struct PciBar
{
	union {
		void * address;
		uint16 port;
	} u;
	uint64 size;
	uint32 flags;
} PciBar;

typedef struct PciDeviceInfo
{
	uint16 vendorId;
	uint16 deviceId;
	uint8  classCode;
	uint8  subclass;
	uint8  progIntf;
} PciDeviceInfo;

typedef struct PciDevice
{
	PciDeviceInfo info;
	uint32        id;
} PciDevice;

// TODO use this
typedef struct PciDriver
{
	void (*init)(uint32 id, PciDeviceInfo *info);
} PciDriver;

uint8 pci_read8(uint32 id, uint32 reg);
uint16 pci_read16(uint32 id, uint32 reg);
uint32 pci_read32(uint32 id, uint32 reg);

void pci_write8(uint32 id, uint32 reg, uint8 data);
void pci_write16(uint32 id, uint32 reg, uint16 data);
void pci_write32(uint32 id, uint32 reg, uint32 data);

void pci_get_bar(PciBar *bar, uint32 id, uint32 index);
uint8 pci_get_interrupt_line(uint32 id);

void pci_init();
