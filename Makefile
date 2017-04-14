CC=gcc
ASMC=nasm
OPTIMIZATION=0
CFLAGS=-m32 -Wall -ffreestanding -fno-builtin -nostdlib -nostdinc -Iinclude -O$(OPTIMIZATION) -g

QEMUCMD=qemu-system-x86_64 -kernel $(BINARY) -hda hda.img


SRC_DIR=src
INC_DIR=include
OBJ_DIR=build
BIN_DIR=bin

SOURCES_C := $(shell find $(SRC_DIR) -name '*.c')
SOURCES_ASM := $(shell find $(SRC_DIR) -name '*.asm')

OBJECTS_C := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.co,$(SOURCES_C))
OBJECTS_ASM := $(patsubst $(SRC_DIR)/%.asm,$(OBJ_DIR)/%.ao,$(SOURCES_ASM))

KERNEL_NAME=AIJAK
BINARY=$(BIN_DIR)/$(KERNEL_NAME)

.PHONY: clean test debug debugger

$(BINARY): $(OBJECTS_C) $(OBJECTS_ASM)
	@if [ ! -d $(dir $@) ]; then mkdir -p $(dir $@) ; fi
	ld -m elf_i386 -T link.ld -o $(BINARY) $(OBJECTS_ASM) $(OBJECTS_C)

$(OBJ_DIR)/%.ao: $(SRC_DIR)/%.asm
	@if [ ! -d $(dir $@) ]; then mkdir -p $(dir $@) ; fi
	$(ASMC) -f elf32 $< -o $@

$(OBJ_DIR)/%.co: $(SRC_DIR)/%.c $(wildcard $(INC_DIR)/%.h)
	@if [ ! -d $(dir $@) ]; then mkdir -p $(dir $@) ; fi
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@-rm -r $(BIN_DIR) $(OBJ_DIR)

test:
	$(QEMUCMD)

debug:
	objcopy --only-keep-debug $(BINARY) debug.sym
	$(QEMUCMD) -S -s
debugger:
	gdb -s debug.sym -ex "target remote localhost:1234"
