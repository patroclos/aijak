#include <display/display.h>
#include <display/textmode/dispi_textmode.h>
#include <drivers/ata.h>
#include <drivers/intel_e1000.h>
#include <drivers/keyboard.h>
#include <drivers/pci.h>
#include <fs/vfs.h>
#include <interrupt/irq.h>
#include <interrupt/isr.h>
#include <kshell.h>
#include <memory/memory.h>
#include <mutex.h>
#include <net/arp.h>
#include <net/intf.h>
#include <pit.h>
#include <stdlib/format.h>
#include <stdlib/stdarg.h>
#include <string.h>
#include <system.h>
#include <tasking.h>
#include <util.h>
#include <vga.h>

MODULE("KERNEL-MAIN");

extern uint32 kernel_end;
extern uint32 kernel_base;

void late_init()
{

	int pid = 0;
	pid     = START("keyboard", keyboard_init);
	pid     = START("vfsinit", vfs_init);
	pid     = START("devicemm", device_init);
	while (is_pid_running(pid))
		schedule_noirq();
	START_AND_WAIT("ata_init", ata_init);
	START_AND_WAIT("pci", pci_init);
	//START("vga", vga_init);

	kshell_init();
	// TODO implement way to get pci device by vendor and device ids

	while (true)
		kprintf("%c", keyboard_get_key());
	schedule_noirq();
}

void kmain()
{
	display_set_current(display_register(textmode_init()));

	mprint("AIJAK up and running\n");
	mm_init(kernel_end);

	idt_init();
	irq_install();

	pit_init();

	paging_init();

	tasking_init((uint32)&late_init);

	for (;;)
	{
		asm("hlt");
	}
}
