AS = nasm
CC = gcc
LD = ld
OBJCOPY = objcopy
QEMU = qemu-system-x86_64
GRUB_MKRESCUE = grub-mkrescue

# Build Directory
BUILD_DIR = build

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

# Link Kernel
kernel.elf: $(BUILD_DIR)/kernel/arch/x86_64/boot.o $(BUILD_DIR)/kernel/kernel.o $(BUILD_DIR)/kernel/drivers/console.o $(BUILD_DIR)/kernel/arch/x86_64/interrupt_stubs.o $(BUILD_DIR)/kernel/arch/x86_64/interrupts.o $(BUILD_DIR)/kernel/drivers/keyboard.o
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
