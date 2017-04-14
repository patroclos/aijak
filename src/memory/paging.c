#include <memory/memory.h>

#include <display/display.h>

MODULE("PAGE");

static uint32 *page_directory = 0;
static uint32  page_dir_loc   = 0;
static uint32 *last_page      = 0;

// paging will be really simple
// reserve 0-8mb for kernel stuff
// heap will be from approx 1-4mb
// and paging stuff will be from 4mb

void paging_map_virtual_to_phys(uint32 virt, uint32 phys)
{
	uint16 id = virt >> 22;
	int32  i;
	for (i = 0; i < 1024; i++)
	{
		last_page[i] = phys | 3;
		phys += 4096;
	}
	page_directory[id] = ((uint32)last_page) | 3;
	last_page += 4096;
	//mprint("Mapping 0x%x (%d) to 0x%x-0x%x\n", virt, id, phys-4096*1024,phys);
}

void paging_enable()
{
	asm volatile("mov %%eax, %%cr3"
	             :
	             : "a"(page_dir_loc));
	asm volatile("mov %cr0, %eax");
	asm volatile("orl $0x80000000, %eax");
	asm volatile("mov %eax, %cr0");
}

void paging_init()
{
	mprint("Setting up paging\n");
	page_directory = (uint32 *)0x400000;
	page_dir_loc   = (uint32)page_directory;
	last_page      = (uint32 *)0x404000;
	int32 i;
	for (i                = 0; i < 1024; i++)
		page_directory[i] = 0 | 2;

	// identity map
	for(i=0;i<1024;i++)
		paging_map_virtual_to_phys(i*0x400000,i*0x400000);

	//paging_map_virtual_to_phys(0x0, 0x0);
	//paging_map_virtual_to_phys(0x400000, 0x400000);
	paging_enable();
	mprint("Paging was successfully enabled!\n");
}
