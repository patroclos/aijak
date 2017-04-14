#include <drivers/intel_e1000.h>

#include <display/display.h>
#include <interrupt/irq.h>
#include <memory/memory.h>
#include <memory/mmioutils.h>
#include <net/addr.h>
#include <net/buf.h>
#include <net/eth.h>
#include <net/intf.h>
#include <system.h>
#include <tasking.h>

MODULE("i217");

#define RX_DESC_COUNT 32
#define TX_DESC_COUNT 8

#define PACKET_SIZE 2048

typedef struct RecvDesc
{
	volatile uint64 addr;
	volatile uint16 len;
	volatile uint16 checksum;
	volatile uint8  status;
	volatile uint8  errors;
	volatile uint16 special;
} __attribute__((packed)) RecvDesc;

// receive status
#define RSTA_DD (1 << 0) // descriptor done
#define RSTA_EOP (1 << 1) // end of packet
#define RSTA_IXSM (1 << 2) // ignore checksum indication
#define RSTA_VP (1 << 3) // packet is 802.10
#define RSTA_TCPCS (1 << 5) // tcp checksum calculated on packet
#define RSTA_IPCS (1 >> 6) // ip checksum calculated on packet
#define RSTA_PIF (1 << 7) // passed in-exact filter

typedef struct TransDesc
{
	volatile uint64 addr;
	volatile uint16 len;
	volatile uint8  cso;
	volatile uint8  cmd;
	volatile uint8  status;
	volatile uint8  css;
	volatile uint16 special;
} __attribute__((packed)) TransDesc;

// transmit command
#define CMD_EOP (1 << 0) // end of packet
#define CMD_IFCS (1 << 1) // insert fcs
#define CMD_IC (1 << 2) // insert checksum
#define CMD_RS (1 << 3) // report status
#define CMD_RPS (1 << 4) // report packet sent
#define CMD_VLE (1 << 6) // vlan packet enable
#define CMD_IDE (1 << 7) // interrupt delay enable

// transmit status
#define TSTA_DD (1 << 0) // descriptor done
#define TSTA_EC (1 << 1) // excess collisions
#define TSTA_LC (1 << 2) // late collision
#define LSTA_TU (1 << 3) // transmit underrun

// device state

typedef struct EthIntelDevice
{
	uint8 *    mmioAddr;
	uint32     rxRead;
	uint32     txWrite;
	RecvDesc * rxDescs;
	TransDesc *txDescs;
	NetBuf *   rxBufs[RX_DESC_COUNT];
	NetBuf *   txBufs[TX_DESC_COUNT];
} EthIntelDevice;

EthIntelDevice s_device;
NetIntf *      intf;

// registers

#define REG_CTRL 0x0000 // Device Control
#define REG_EERD 0x0014 // EEPROM Read
#define REG_ICR 0x00c0 // Interrupt Cause Read
#define REG_IMS 0x00d0 // Interrupt Mask Set/Read
#define REG_RCTL 0x0100 // Receive Control
#define REG_TCTL 0x0400 // Transmit Control
#define REG_RDBAL 0x2800 // Receive Descriptor Base Low
#define REG_RDBAH 0x2804 // Receive Descriptor Base High
#define REG_RDLEN 0x2808 // Receive Descriptor Length
#define REG_RDH 0x2810 // Receive Descriptor Head
#define REG_RDT 0x2818 // Receive Descriptor Tail
#define REG_TDBAL 0x3800 // Transmit Descriptor Base Low
#define REG_TDBAH 0x3804 // Transmit Descriptor Base High
#define REG_TDLEN 0x3808 // Transmit Descriptor Length
#define REG_TDH 0x3810 // Transmit Descriptor Head
#define REG_TDT 0x3818 // Transmit Descriptor Tail
#define REG_MTA 0x5200 // Multicast Table Array
#define REG_RAL 0x5400 // Receive Address Low
#define REG_RAH 0x5404 // Receive Address High

// ------------------------------------------------------------------------------------------------
// Control Register

#define CTRL_SLU (1 << 6) // Set Link Up

// ------------------------------------------------------------------------------------------------
// EERD Register

#define EERD_START 0x0001 // Start Read
#define EERD_DONE 0x0010 // Read Done
#define EERD_ADDR_SHIFT 8
#define EERD_DATA_SHIFT 16

// ------------------------------------------------------------------------------------------------
// RCTL Register

#define RCTL_EN (1 << 1) // Receiver Enable
#define RCTL_SBP (1 << 2) // Store Bad Packets
#define RCTL_UPE (1 << 3) // Unicast Promiscuous Enabled
#define RCTL_MPE (1 << 4) // Multicast Promiscuous Enabled
#define RCTL_LPE (1 << 5) // Long Packet Reception Enable
#define RCTL_LBM_NONE (0 << 6) // No Loopback
#define RCTL_LBM_PHY (3 << 6) // PHY or external SerDesc loopback
#define RTCL_RDMTS_HALF (0 << 8) // Free Buffer Threshold is 1/2 of RDLEN
#define RTCL_RDMTS_QUARTER (1 << 8) // Free Buffer Threshold is 1/4 of RDLEN
#define RTCL_RDMTS_EIGHTH (2 << 8) // Free Buffer Threshold is 1/8 of RDLEN
#define RCTL_MO_36 (0 << 12) // Multicast Offset - bits 47:36
#define RCTL_MO_35 (1 << 12) // Multicast Offset - bits 46:35
#define RCTL_MO_34 (2 << 12) // Multicast Offset - bits 45:34
#define RCTL_MO_32 (3 << 12) // Multicast Offset - bits 43:32
#define RCTL_BAM (1 << 15) // Broadcast Accept Mode
#define RCTL_VFE (1 << 18) // VLAN Filter Enable
#define RCTL_CFIEN (1 << 19) // Canonical Form Indicator Enable
#define RCTL_CFI (1 << 20) // Canonical Form Indicator Bit Value
#define RCTL_DPF (1 << 22) // Discard Pause Frames
#define RCTL_PMCF (1 << 23) // Pass MAC Control Frames
#define RCTL_SECRC (1 << 26) // Strip Ethernet CRC

// Buffer Sizes
#define RCTL_BSIZE_256 (3 << 16)
#define RCTL_BSIZE_512 (2 << 16)
#define RCTL_BSIZE_1024 (1 << 16)
#define RCTL_BSIZE_2048 (0 << 16)
#define RCTL_BSIZE_4096 ((3 << 16) | (1 << 25))
#define RCTL_BSIZE_8192 ((2 << 16) | (1 << 25))
#define RCTL_BSIZE_16384 ((1 << 16) | (1 << 25))

// ------------------------------------------------------------------------------------------------
// TCTL Register

#define TCTL_EN (1 << 1) // Transmit Enable
#define TCTL_PSP (1 << 3) // Pad Short Packets
#define TCTL_CT_SHIFT 4 // Collision Threshold
#define TCTL_COLD_SHIFT 12 // Collision Distance
#define TCTL_SWXOFF (1 << 22) // Software XOFF Transmission
#define TCTL_RTLC (1 << 24) // Re-transmit on Late Collision

// ------------------------------------------------------------------------------------------------
//
//

uint16 eepromRead(uint8 *mmioAddr, uint8 eepromAddr)
{
	mmio_write32(mmioAddr + REG_EERD, EERD_START | (eepromAddr << EERD_ADDR_SHIFT));

	uint32 val;
	do
	{
		val = mmio_read32(mmioAddr + REG_EERD);
	} while (~val & EERD_DONE);
	return val >> EERD_DATA_SHIFT;
}

void eth_intel_poll(NetIntf *intf)
{
	RecvDesc *desc = &s_device.rxDescs[s_device.rxRead];

	while (desc->status & RSTA_DD)
	{
		if (desc->errors)
		{
			mprint("Packet Error: (0x%x)\n", desc->errors);
		}
		else
		{
			NetBuf *buf = s_device.rxBufs[s_device.rxRead];
			buf->end    = buf->start + desc->len;

			eth_recv(intf, buf);

			net_release_buf(buf);
			buf        = net_alloc_buf();
			desc->addr = (uint64)(uintptr_t)buf->start;
		}

		desc->status = 0;

		mmio_write32(s_device.mmioAddr + REG_RDT, s_device.rxRead);

		s_device.rxRead = (s_device.rxRead + 1) & (RX_DESC_COUNT - 1);
		desc            = &s_device.rxDescs[s_device.rxRead];
	}
}

void eth_intel_send(NetBuf *buf)
{
	TransDesc *desc   = &s_device.txDescs[s_device.txWrite];
	NetBuf *   oldBuf = s_device.txBufs[s_device.txWrite];

	// wait until packet is sent
	while (!(desc->status & 0xf))
		_sleep(1);

	// free packet that was sent with this descriptor

	if (oldBuf)
		net_release_buf(oldBuf);

	// write new tx descriptor
	desc->addr                        = (uint64)(uintptr_t)buf->start;
	desc->len                         = buf->end - buf->start;
	desc->cmd                         = (CMD_EOP | CMD_IFCS | CMD_RS);
	desc->status                      = 0;
	s_device.txBufs[s_device.txWrite] = buf;

	s_device.txWrite = (s_device.txWrite + 1) & (TX_DESC_COUNT - 1);
	mmio_write32(s_device.mmioAddr + REG_TDT, s_device.txWrite);
}

void eth_intel_irq(struct regs *r)
{
	uint32 cause = mmio_read32(s_device.mmioAddr + REG_ICR);
	//mprint("IRQ#%d fired.. cause: %b\n", r->int_no - 32, cause);

	if (cause & 0x80)
	{
		eth_intel_poll(intf);
	}

	mmio_write32(s_device.mmioAddr + 0xc8, 0x00);
	send_eoi(r->int_no - 32);
}

// ----------------------------INTEL INIT -----------------------------------------

void eth_intel_init(uint32 id, PciDeviceInfo *info)
{
	// check device supported
	if (info->vendorId != 0x8086)
		return;
	if (info->deviceId != 0x100e && info->deviceId != 0x1503)
		return;

	mprint("Initializing Intel Gigabit Ethernet\n");

	// base io address
	PciBar bar;
	pci_get_bar(&bar, id, 0);

	// only memory mapped io supported
	if (bar.flags & PCI_BAR_IO)
		return;

	uint8 *mmioAddr   = (uint8 *)bar.u.address;
	s_device.mmioAddr = mmioAddr;

	// enable bus mastering
	uint16 cmd = pci_read16(id, PCI_CONFIG_COMMAND);
	cmd |= 4;
	pci_write16(id, PCI_CONFIG_COMMAND, cmd);

	// enable interrupts
	//set_idt_gate(IRQ0 + pci_get_interrupt_line(id), (uint32)eth_intel_irq);
	irq_setroutine(pci_get_interrupt_line(id), (uint32)eth_intel_irq);
	mmio_write32(s_device.mmioAddr + REG_IMS, 0x1fffff);
	mmio_write32(s_device.mmioAddr + REG_IMS, 0xff & ~4);
	mmio_read32(s_device.mmioAddr + 0xc0);
	// clear all interrupts
	mmio_read32(mmioAddr + REG_ICR);

	// reading MAC address
	EthAddr localAddr;
	uint32  ral = mmio_read32(mmioAddr + REG_RAL);
	if (ral)
	{
		uint32 rah = mmio_read32(mmioAddr + REG_RAH);

		localAddr.n[0] = (uint8)(ral);
		localAddr.n[1] = (uint8)(ral >> 8);
		localAddr.n[2] = (uint8)(ral >> 16);
		localAddr.n[3] = (uint8)(ral >> 24);
		localAddr.n[4] = (uint8)(rah);
		localAddr.n[5] = (uint8)(rah >> 8);
	}
	else
	{
		uint16 mac01 = eepromRead(mmioAddr, 0);
		uint16 mac23 = eepromRead(mmioAddr, 1);
		uint16 mac45 = eepromRead(mmioAddr, 2);

		localAddr.n[0] = (uint8)(mac01);
		localAddr.n[1] = (uint8)(mac01 >> 8);
		localAddr.n[2] = (uint8)(mac23 >> 16);
		localAddr.n[3] = (uint8)(mac23 >> 24);
		localAddr.n[4] = (uint8)(mac45);
		localAddr.n[5] = (uint8)(mac45 >> 8);
	}

	char macStr[ETH_ADDR_STRING_SIZE] = { 0 };
	addr_eth_tostr(macStr, ETH_ADDR_STRING_SIZE, &localAddr);
	mprint("MAC: %s\n", macStr);

	// set link up
	mmio_write32(mmioAddr + REG_CTRL, mmio_read32(mmioAddr + REG_CTRL) | CTRL_SLU);

	// clear multicast table array
	int i;
	for (i = 0; i < 128; i++)
		mmio_write32(mmioAddr + REG_MTA + (i * 4), 0);

	// clear previous memory
	if (s_device.rxDescs)
		pfree(s_device.rxDescs);
	if (s_device.txDescs)
		pfree(s_device.txDescs);
	// allocate memory
	RecvDesc * rxDescs = (RecvDesc *)pmalloc(RX_DESC_COUNT * sizeof(RecvDesc));
	TransDesc *txDescs = (TransDesc *)pmalloc(TX_DESC_COUNT * sizeof(TransDesc));

	s_device.rxDescs = rxDescs;
	s_device.txDescs = txDescs;

	// receive setup
	for (i = 0; i < RX_DESC_COUNT; i++)
	{
		NetBuf *buf = net_alloc_buf();

		s_device.rxBufs[i] = buf;

		RecvDesc *rxDesc = rxDescs + i;
		rxDesc->addr     = (uint64)(uintptr_t)buf->start;
		rxDesc->status   = 0;
	}

	s_device.rxRead = 0;

	mmio_write32(mmioAddr + REG_RDBAL, (uintptr_t)rxDescs);
	mmio_write32(mmioAddr + REG_RDBAH, (uintptr_t)rxDescs >> 32);
	mmio_write32(mmioAddr + REG_RDLEN, RX_DESC_COUNT * 16);
	mmio_write32(mmioAddr + REG_RDH, 0);
	mmio_write32(mmioAddr + REG_RDT, RX_DESC_COUNT - 1);
	mmio_write32(mmioAddr + REG_RCTL, RCTL_EN | RCTL_SBP | RCTL_UPE | RCTL_MPE | RCTL_LBM_NONE | RTCL_RDMTS_HALF | RCTL_BAM | RCTL_SECRC | RCTL_BSIZE_2048);

	// transmit setup
	TransDesc *txDesc = txDescs;
	TransDesc *txEnd  = txDescs + TX_DESC_COUNT;
	memset(txDesc, 0, TX_DESC_COUNT * 16);

	for (; txDesc != txEnd; txDesc++)
		txDesc->status = TSTA_DD; // mark descriptor as complete

	s_device.txWrite = 0;

	mmio_write32(mmioAddr + REG_TDBAL, (uintptr_t)txDescs);
	mmio_write32(mmioAddr + REG_TDBAH, (uintptr_t)txDescs >> 32);
	mmio_write32(mmioAddr + REG_TDLEN, TX_DESC_COUNT * 16);
	mmio_write32(mmioAddr + REG_TDH, 0);
	mmio_write32(mmioAddr + REG_TDT, 0);
	mmio_write32(mmioAddr + REG_TCTL,
	    TCTL_EN
	        | TCTL_PSP
	        | (15 << TCTL_CT_SHIFT)
	        | (64 << TCTL_COLD_SHIFT)
	        | TCTL_RTLC);

	//mmio_write32(mmioAddr + REG_TCTL, 0b0110000000000111111000011111010);
	//mmio_write32(mmioAddr+0x410,0x0060200a);

	intf                = net_intf_create();
	intf->ethAddr       = localAddr;
	intf->ipAddr.u.bits = 0x7b02a8c0; //g_nullIPv4Addr;
	intf->name          = "eth";
	intf->poll          = eth_intel_poll;
	intf->send          = eth_send_intf;
	intf->devSend       = eth_intel_send;

	net_intf_add(intf);
}
