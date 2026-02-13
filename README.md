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
- **Physical Memory Manager (PMM)**: Bitmap-based 4KB frame allocator initialized from Multiboot2 memory map.
- **Memory Debug Command (`meminfo`)**: Reports total/used/free memory and runs a small allocate/free leak check.
- **Serial Logging Support**: COM1 initialization for easier debugging alongside VGA output.
- **Read-Only Tar Filesystem (`tarfs`)**: Loads an embedded `initrd.tar` at boot and exposes file listing/reading from kernel shell.
- **Shell Commands (`ls`, `cat`)**: Basic command parser with argument validation and user-facing error messages.

---

##  Project Structure

```text
.
├── boot/                # Bootloader configuration (grub.cfg)
├── kernel/              # Core Kernel Source
│   ├── arch/x86_64/     # Hardware-specific code (boot, interrupts, ports, Multiboot defs)
│   ├── drivers/         # Hardware drivers (Console, Keyboard, Serial)
│   ├── fs/              # Read-only tar filesystem implementation
│   ├── lib/             # Common types and helpers (meminfo command)
│   └── mm/              # Memory management (Physical Memory Manager)
├── initrd/              # Files packed into initrd.tar (filesystem payload)
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

At the kernel prompt, run:
```text
meminfo
ls
cat hello.txt
cat docs/guide.txt
```
to inspect memory and read files from the embedded filesystem.

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
- [Physical Memory Manager Explained (Thai)](file:///home/me/myOS/PMM_EXPLAINED_TH.md)

---

## Boot Flow (Current)

1. GRUB loads the kernel via Multiboot2.
2. `boot.asm` validates environment, sets page tables, and switches to 64-bit long mode.
3. `kernel_main` initializes console, serial, PMM, interrupts, and keyboard.
4. Kernel initializes tarfs from embedded `initrd.tar` and runs a filesystem self-test.
5. The kernel enters an interrupt-driven loop (`hlt`) and accepts shell commands.

---

## Storage & Filesystem (tarfs)

### Design

- Filesystem type is **read-only USTAR (`tar`)**.
- Archive is built at compile time from `initrd/` into `build/initrd.tar`.
- `objcopy` converts the archive into an ELF object (`build/initrd.tar.o`).
- Linker includes the object into `kernel.elf`.
- Kernel accesses archive bytes through linker symbols:
  - `_binary_build_initrd_tar_start`
  - `_binary_build_initrd_tar_end`

### Runtime Flow

1. `kernel_main` computes archive size from linker symbols.
2. `tarfs_init()` validates archive presence and USTAR magic.
3. `tarfs_ls()` iterates TAR headers and prints entries.
4. `tarfs_cat(path)` finds a matching entry and prints file content.

### Current Scope

- Read-only access only (no create/write/delete).
- Flat `ls` output over archive entries (shows stored paths such as `docs/guide.txt`).
- `cat` supports simple path lookup (normalized for `./` and trailing `/`).

---

## Shell Commands

### `meminfo`

- Shows PMM stats:
  - total memory
  - used memory
  - free memory
- Runs a small allocate/free leak check.

### `ls`

- Lists entries from tarfs archive.
- Example output:
  - `hello.txt`
  - `docs/`
  - `docs/guide.txt`

### `cat <path>`

- Prints file contents from tarfs.
- Examples:
  - `cat hello.txt`
  - `cat docs/guide.txt`

---

## Error Handling (Implemented)

- `cat` without argument:
  - `cat: missing file operand`
- `cat <missing-file>`:
  - `cat: <name>: no such file`
- `cat <directory>`:
  - `cat: <name>: is a directory`
- `ls` with unsupported arguments:
  - `ls: path arguments are not supported yet`
- `meminfo` with unexpected arguments:
  - `meminfo: this command takes no arguments`
- Filesystem unavailable/corrupt archive:
  - `ls: filesystem not ready`
  - `cat: filesystem not ready`
  - boot-time diagnostics from `tarfs_init()`

---

## Build Pipeline for initrd

`Makefile` automates the storage pipeline:

1. Collect files from `initrd/`.
2. Build `build/initrd.tar` with:
```bash
tar --format=ustar -cf build/initrd.tar -C initrd .
```
3. Convert archive to object:
```bash
objcopy -I binary -O elf64-x86-64 -B i386:x86-64 build/initrd.tar build/initrd.tar.o
```
4. Link `build/initrd.tar.o` into `kernel.elf`.

---

## License

This project is open-source and available under the [MIT License](LICENSE).
