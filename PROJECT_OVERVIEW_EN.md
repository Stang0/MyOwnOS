# MyOS Project Overview

This is a small Operating System project written in Assembly (x86_64) and C++, focusing on understanding fundamental computer operations.

## Project Structure

The files are organized as follows:

### 1. `boot/` (Bootloader Configuration)
*   **`grub.cfg`**: GRUB bootloader configuration file. It tells GRUB where to load our Kernel from and what commands to use.

### 2. `kernel/` (Core System)
The main files controlling the entire operation:
*   **`kernel.cpp`**: The OS Main Entry Point in C++. It initializes various systems and runs the main loop.

#### `kernel/arch/x86_64/` (Hardware Specifics)
*   **`boot.asm`**:  
    *   First-stage Assembly code that runs upon boot.
    *   Takes over from GRUB (which is in 32-bit Protected Mode).
    *   **Main Duty**: Sets up Page Tables, GDT, and switches CPU to **64-bit Long Mode** before jumping to `kernel_main` in C++.
*   **`interrupts.cpp` / `.hpp`**: Manages IDT (Interrupt Descriptor Table) and PIC (Programmable Interrupt Controller) so the CPU can handle hardware interrupts.
*   **`interrupt_stubs.asm`**: Small Assembly stubs that save CPU state (Registers) before calling C++ interrupt handlers.
*   **`ports.hpp`**: Helper functions for communicating with hardware via I/O Ports (`inb`, `outb`).

#### `kernel/drivers/` (Device Drivers)
*   **`console.cpp` / `.hpp`**:
    *   **VGA Console**: Controls the Text Mode screen (0xB8000).
    *   Supports printing characters, changing colors, scrolling, and cursor movement.
*   **`keyboard.cpp` / `.hpp`**:
    *   **Keyboard Driver**: Receives keypresses from IRQ1 (Port 0x60).
    *   Converts Scan Codes to ASCII.
    *   Supports **Shift**, **Caps Lock**, **Backspace**, and numbers/symbols.

#### `kernel/lib/` (Basic Libraries)
*   **`types.h`**: Defines basic variable types like `uint8_t`, `uint64_t` to be consistent with standard OS development.

### 3. `scripts/` (Build Scripts)
*   **`linker.ld`**: Tells the Linker how to arrange our code in memory (e.g., start at Address 1MB and place the Multiboot Header at the very beginning).

### Other Files at Root
*   **`Makefile`**: Command set for compiling and building the entire project. Just run `make` or `make run`, and the system handles everything.

---

## Execution Flow

1.  **Boot**: BIOS -> GRUB -> `boot.asm` (64-bit Setup) -> `kernel_main`
2.  **Kernel Init**:
    *   `console.init()`: Prepares the screen.
    *   `init_interrupts()`: Configures IDT/PIC.
    *   `irq_install_handler()`: Installs the Keyboard Driver.
    *   `sti`: Enables Interrupts.
3.  **Runtime**: The OS enters an infinite loop, waiting for Keyboard Interrupts. When a key is pressed, `keyboard_callback` executes and displays the character.

---

## Tools Used

This project uses standard tools for OS development:

*   **GCC (g++)**: C++ Compiler. Uses special flags like `-ffreestanding` to indicate Bare Metal environment (no host OS).
*   **NASM**: Assembler for converting Assembly code (.asm) to Machine Code.
*   **LD (GNU Linker)**: Links Object files (.o) together as specified in the Linker Script (`linker.ld`).
*   **GRUB**: Standard Bootloader used to load our Kernel.
*   **QEMU**: Computer Emulator. Allows running and testing the OS without installing on real hardware.
*   **Make**: Build automation tool that reads instructions from `Makefile`.
*   **xorriso**: Program for creating bootable ISO images.
