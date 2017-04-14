#pragma once

#include <drivers/pci.h>
#include <net/intf.h>
#include <net/buf.h>
#include <types.h>

void eth_intel_init(uint32 id, PciDeviceInfo *info);
void eth_intel_send(NetBuf *buf);
void eth_intel_poll(NetIntf *intf);
