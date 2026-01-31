# MyOwnOS 

MyOwnOS is a toy 64-bit Operating System built from scratch using Assembly (NASM) and C++. This project is designed to explore the low-level details of computer architecture, boot processes, and kernel development.

![OS Preview](https://img.shields.io/badge/Architecture-x86__64-blue)
![License](https://img.shields.io/badge/License-MIT-green)

---

##  Key Features

- **64-bit Long Mode**: Successfully transitions from 32-bit Protected Mode to 64-bit Long Mode using a custom bootloader.
- **4-Level Paging**: Implements identity mapping for the first 2MB of memory with a 4-level paging hierarchy (PML4, PDP, PD, PT).
- **VGA Text Mode Driver**: A modular console driver with support for:
    - Printing characters and strings.
    - Custom foreground/background colors.
    - Automated scrolling and hardware cursor management.
    - Kernel Panic screen.
- **Interrupt Handling**: Fully configured IDT (Interrupt Descriptor Table) and PIC remapping.
- **Keyboard Driver**: PS/2 keyboard support with Scan Code translation, Shift, Caps Lock, and Backspace functionality.
- **Multiboot 2 Compliant**: Boots seamlessly using the GRUB bootloader.

---

##  Project Structure

```text
.
├── boot/                # Bootloader configuration (grub.cfg)
├── kernel/              # Core Kernel Source
│   ├── arch/x86_64/     # Hardware-specific code (ASM/CPP)
│   ├── drivers/         # Hardware drivers (Console, Keyboard)
│   └── lib/             # Common types and utilities
├── build/               # Compiled object files (auto-generated)
├── scripts/             # Linker scripts
├── Makefile             # Build system automation
└── CPU_BASICS_EN.md     # Learning documentation (English)
```

---

## Prerequisites

To build and run this OS, you will need the following tools installed on a Linux environment:

- **Compiler**: `gcc` (with g++ for C++)
- **Assembler**: `nasm`
- **Linker**: `ld`
- **Bootloader**: `grub-pc` or `grub-common`
- **ISO Creation**: `xorriso` and `grub-mkrescue`
- **Emulator**: `qemu-system-x86_64`

---

## Getting Started

### 1. Build and Run
The included `Makefile` handles the entire build process, including compiling assembly stubs, C++ kernel, and linking them into a bootable ISO.

To build and launch in QEMU:
```bash
make run
```

To just compile the project:
```bash
make
```

To clean up build artifacts:
```bash
make clean
```

---

## Learn More

This project also includes detailed documentation for those who want to understand the underlying architecture:
- [CPU Basics (English)](file:///home/me/myOS/CPU_BASICS_EN.md)
- [CPU Basics (Thai)](file:///home/me/myOS/CPU_BASICS_TH.md)
- [Project Overview (English)](file:///home/me/myOS/PROJECT_OVERVIEW_EN.md)

---

## License

This project is open-source and available under the [MIT License](LICENSE).
