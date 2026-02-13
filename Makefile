AS = nasm
CC = gcc
LD = ld
OBJCOPY = objcopy
QEMU = qemu-system-x86_64
GRUB_MKRESCUE = grub-mkrescue
TAR = tar

# Build Directory
BUILD_DIR = build
INITRD_DIR = initrd
INITRD_TAR = $(BUILD_DIR)/initrd.tar
INITRD_OBJ = $(BUILD_DIR)/initrd.tar.o

# Include paths
INCLUDES = -Ikernel/lib -Ikernel/drivers -Ikernel/arch/x86_64 -Ikernel

CFLAGS  = -std=c++17 -ffreestanding -m64 -g -Wall -Wextra -fno-exceptions -fno-rtti -fno-stack-protector -fno-pie $(INCLUDES)
ASFLAGS = -felf64
LDFLAGS = -T scripts/linker.ld -nostdlib -static -z max-page-size=0x1000

all: os.iso

# Kernel Objects
$(BUILD_DIR)/kernel/kernel.o: kernel/kernel.cpp
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

# Arch Objects
$(BUILD_DIR)/kernel/arch/x86_64/boot.o: kernel/arch/x86_64/boot.asm
	@mkdir -p $(@D)
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/kernel/arch/x86_64/interrupt_stubs.o: kernel/arch/x86_64/interrupt_stubs.asm
	@mkdir -p $(@D)
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/kernel/arch/x86_64/interrupts.o: kernel/arch/x86_64/interrupts.cpp
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

# Driver Objects
$(BUILD_DIR)/kernel/drivers/console.o: kernel/drivers/console.cpp
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/kernel/drivers/keyboard.o: kernel/drivers/keyboard.cpp
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/kernel/mm/pmm.o: kernel/mm/pmm.cpp
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/kernel/lib/helpers.o: kernel/lib/helpers.cpp
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/kernel/fs/tarfs.o: kernel/fs/tarfs.cpp
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(INITRD_TAR): $(shell find $(INITRD_DIR) -type f)
	@mkdir -p $(@D)
	$(TAR) --format=ustar -cf $@ -C $(INITRD_DIR) .

$(INITRD_OBJ): $(INITRD_TAR)
	$(OBJCOPY) -I binary -O elf64-x86-64 -B i386:x86-64 $< $@

# Link Kernel
kernel.elf: $(BUILD_DIR)/kernel/arch/x86_64/boot.o $(BUILD_DIR)/kernel/kernel.o $(BUILD_DIR)/kernel/drivers/console.o $(BUILD_DIR)/kernel/arch/x86_64/interrupt_stubs.o $(BUILD_DIR)/kernel/arch/x86_64/interrupts.o $(BUILD_DIR)/kernel/drivers/keyboard.o $(BUILD_DIR)/kernel/mm/pmm.o $(BUILD_DIR)/kernel/lib/helpers.o $(BUILD_DIR)/kernel/fs/tarfs.o $(INITRD_OBJ)
	$(LD) $(LDFLAGS) -o $@ $^

kernel.bin: kernel.elf
	$(OBJCOPY) -O binary $< $@

os.iso: kernel.elf boot/grub.cfg
	mkdir -p isodir/boot/grub
	cp kernel.elf isodir/boot/
	cp boot/grub.cfg isodir/boot/grub/
	$(GRUB_MKRESCUE) -o $@ isodir

run: os.iso
	$(QEMU) -cdrom os.iso -serial stdio

clean:
	rm -rf $(BUILD_DIR) *.elf *.bin os.iso isodir/

.PHONY: all run clean
