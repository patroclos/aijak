CC=gcc
CFLAGS=-m32 -c -Wall -ffreestanding -O0

CMODULES=system string screen keyboard util idt isr
MODULES=kernelasm kernel
MODULEOBJS=$(addprefix obj/,$(addsuffix .o,$(MODULES)))
KERNEL_NAME=iso/boot/kernel.bin

OBJS=$(addprefix obj/include/,$(addsuffix .o,$(CMODULES)))

$(KERNEL_NAME): $(OBJS) $(MODULEOBJS)
	ld -m elf_i386 -T link.ld -o $(KERNEL_NAME) $(MODULEOBJS) $(OBJS)

all: $(KERNEL_NAME) build-iso

obj/kernelasm.o: kernel.asm | obj/
	nasm -f elf32 kernel.asm -o $(addprefix obj/,kernelasm.o)

obj/kernel.o: kernel.c | obj/
	$(CC) $(CFLAGS) kernel.c -o obj/kernel.o

obj/include/%.o: include/%.c include/%.h | obj/ obj/include
	$(CC) $(CFLAGS) $< -o $@

obj/ obj/include:
	@mkdir $@

build-iso: aijak.iso

aijak.iso: $(KERNEL_NAME)
	grub2-mkrescue -o aijak.iso iso

clean:
	@-rm -r obj/ $(KERNEL_NAME)

test:
	qemu-system-x86_64 -kernel $(KERNEL_NAME)
