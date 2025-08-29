# ========= CONFIG =========
ARCH        := i386
TARGET      := cubebox
BUILD_DIR   := build
BIN_DIR     := $(BUILD_DIR)/bin
OBJ_DIR     := $(BUILD_DIR)/obj
ISO_DIR     := $(BUILD_DIR)/iso
SRC_DIR     := src
CFG_DIR     := config

CFLAGS      :=	-std=gnu99 -ffreestanding -O2 -Wall -Wextra -m32									\
				-fno-omit-frame-pointer -fno-optimize-sibling-calls -mno-omit-leaf-frame-pointer 	\
				-D _BUILD_INSIDERS -D _OPT_SEED_ENTROPY_RTC -D _OPT_SEED_ENTROPY_CPUID 				\
				-I$(SRC_DIR) -I kernel -I drivers -I kernel/common/structures -I kernel/common -I libc -I kernel/core -I shell -I fs -I .

LDFLAGS     := 	-T $(CFG_DIR)/linker.ld -m elf_i386 -nostdlib
ASFLAGS     := 	-felf32

CC          := gcc
LD          := ld
AS          := nasm

# ========= SOURCES =========
C_SRCS      :=	$(shell find $(SRC_DIR) -name "*.c" ! -name "kernel.sym.c")
ASM_SRCS    := 	$(shell find $(SRC_DIR) -name "*.asm")
OBJS        := 	$(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(C_SRCS)) 	\
				$(patsubst $(SRC_DIR)/%.asm,$(OBJ_DIR)/%.o,$(ASM_SRCS)) \
				$(OBJ_DIR)/kernel/core/kernel.sym.c.o

# ========= SYMBOL EXTRACTION =========
SYMBOL_MAP	:= $(BUILD_DIR)/kernel.map
SYM_ENTRIES := $(SRC_DIR)/kernel/core/kernel.sym.entries
SYM_SOURCE 	:= $(SRC_DIR)/kernel/core/kernel.sym.c

# ========= DEFAULT =========
all: $(BIN_DIR)/krnl.bin

# ========= RULES =========
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.asm
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

# First linking before symbol extraction
$(BIN_DIR)/krnl.tmp.elf: $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(LD) $(LDFLAGS) -Map=$(SYMBOL_MAP) -o $@ $(filter-out $(OBJ_DIR)/kernel/core/kernel.sym.c.o,$^)

# Extract symbols from tmp.elf
$(SYM_SOURCE): $(BIN_DIR)/krnl.tmp.elf
	nm -n $< > $(SYMBOL_MAP)

	awk '$$2 == "T" { printf("    { 0x%s, \"%s\" },\n", $$1, $$3); }' $(SYMBOL_MAP) > $(SYM_ENTRIES)
	echo '#include "kernel/core/symbols.h"' > $@
	echo 'symbol_t kernel_symbols[] = {' >> $@
	cat $(SYM_ENTRIES) >> $@
	echo '};' >> $@
	echo 'size_t kernel_symbol_count = sizeof(kernel_symbols) / sizeof(kernel_symbols[0]);' >> $@

# Compile symbol source
$(OBJ_DIR)/kernel/core/kernel.sym.c.o: $(SYM_SOURCE)
	$(CC) $(CFLAGS) -c $< -o $@

# Final linking with symbols
$(BIN_DIR)/krnl.bin: $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(LD) $(LDFLAGS) -o $@ $^

# ========= INITRD & IMGGEN =========
initrd:
	@sh script/initrd.sh

imggen:
	@sh script/imggen.sh

post: initrd imggen
	@echo "DONE!"

# ========= ISO =========
iso: $(BIN_DIR)/krnl.bin
	@mkdir -p $(ISO_DIR)/boot/grub
	cp $(BIN_DIR)/krnl.bin $(ISO_DIR)/boot/kernel.bin
	cp $(CFG_DIR)/grub.cfg $(ISO_DIR)/boot/grub/
	grub-mkrescue -o $(BUILD_DIR)/cubebox.iso $(ISO_DIR)

# ========= QEMU =========
run: iso
	qemu-system-i386 -cdrom $(BUILD_DIR)/cubebox.iso -serial file:serial.log

# ========= CONFIGURATION =========
# config:	
# 	@python3 script/.configure.py
#
# mkconfig:
#	@python3 script/_configure.py
#

configure:
	@python3 script/_configure.py

# ========= CLEAN =========
clean:
	rm -rf $(BUILD_DIR) $(SYM_ENTRIES) $(SYM_SOURCE)

.PHONY: all clean configure initrd imggen iso run post
