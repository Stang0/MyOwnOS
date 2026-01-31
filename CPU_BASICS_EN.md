# x86 Architecture Basics and Assembly Language

This document summarizes the fundamental knowledge required for Kernel development regarding CPU tools and internal mechanisms.

---

## 1. x86 CPU Registers

Registers are high-speed internal memory within the CPU used to store temporary data during processing. In a **64-bit (x86_64)** system, these registers are 64 bits wide.

### General Purpose Registers
Used for calculations and storing memory addresses:

| Name (64-bit) | Main Responsibility |
| :--- | :--- |
| **RAX** | Accumulator: Used for calculation results and function return values. |
| **RBX** | Base: Used as a pointer to data in memory. |
| **RCX** | Counter: Used as a loop counter. |
| **RDX** | Data: Used in I/O operations and multiply/divide calculations. |
| **RSI** | Source Index: Points to the source location for data copying. |
| **RDI** | Destination Index: Points to the destination location for data copying. |
| **RSP** | Stack Pointer: **(Critical)** Points to the current top of the Stack. |
| **RBP** | Base Pointer: Points to the base of the current function's stack frame. |
| **R8 - R15** | Additional general-purpose registers available only in 64-bit mode. |

> **Note:** You can access sub-portions of registers, such as **EAX** (lower 32-bit), **AX** (lower 16-bit), and **AL** (lowest 8-bit).

---

## 2. EFLAGS Register

EFLAGS is a special 32-bit register where each bit acts as a "Flag" to report status or control CPU operations.

### Types of Flags

#### A. Status Flags (Report Calculation Results)
Used after instructions like `cmp`, `add`, `sub`, `test`, etc.:
| Bit | Name | Function |
| :--- | :--- | :--- |
| **0** | **CF (Carry)** | Set if an arithmetic operation generates a carry or borrow out of the most significant bit. |
| **6** | **ZF (Zero)** | Set if the result of the last operation was **0**. |
| **7** | **SF (Sign)** | Set if the result is negative. |
| **11** | **OF (Overflow)** | Set if the result is too large/small to fit in the destination register. |

#### B. Control & System Flags (System Operation Control)
| Bit | Name | Function |
| :--- | :--- | :--- |
| **9** | **IF (Interrupt)** | **Critical:** If 0, the CPU ignores external maskable interrupts (controlled via `cli`/`sti`). |
| **10** | **DF (Direction)** | Controls the direction of string processing (left-to-right or right-to-left). |
| **21** | **ID (ID Flag)** | **Used to check CPUID:** If this bit can be flipped, the CPU supports the `cpuid` instruction. |

---

## 3. Segment Registers

These registers are not for calculations like RAX but identify **"Segments"** or memory access permissions for specific areas:

| Name | Full Name | Current Role in 64-bit |
| :--- | :--- | :--- |
| **CS** | Code Segment | Points to the code currently being executed (defines Privilege Level). |
| **DS** | Data Segment | Points to general data (usually set to 0 in 64-bit). |
| **SS** | Stack Segment | Points to the Stack area (temporary workspace). |
| **ES** | Extra Segment | Additional register for string/data operations. |
| **FS / GS** | Segment F / G | Special registers often used by the OS for Thread Local Storage. |

> **In 64-bit Mode:** The CPU ignores protection for DS, ES, and SS in most cases (Flat Model). Therefore, we typically clear these to **0** immediately upon entering Long Mode for security and consistency.

---

## 4. x86_64 4-Level Paging

In 64-bit mode, the CPU mandates the use of **Paging** to translate Virtual Addresses to Physical Addresses in RAM using 4 levels:

### Table Hierarchy
1.  **PML4 (Page Map Level 4):** The Root table, points to PDP tables.
2.  **PDP (Page Directory Pointer):** Points to Page Directory tables.
3.  **PD (Page Directory):** Points to Page Table tables.
4.  **PT (Page Table):** Points to the actual Physical Page Frame in RAM (4KB size).

### Critical Entry Flags
Each entry in these tables contains bits to define properties, such as:
*   **Bit 0 (Present):** 1 = Page exists in RAM, 0 = Causes a Page Fault if accessed.
*   **Bit 1 (Writable):** 1 = Read/Write allowed, 0 = Read-only.
*   **Bit 2 (User/Supervisor):** 1 = Accessible by user apps, 0 = Kernel only.

---

## 5. Assembly Instructions

Assembly language consists of instructions sent directly to the CPU.

### Data Movement
*   `mov a, b`: Copy value from `b` to `a`.
*   `push a`: Place value `a` onto the top of the Stack.
*   `pop a`: Retrieve value from the top of the Stack into `a`.

### Control Flow
*   `jmp label`: Jump to and execute code at `label` immediately.
*   `call func`: Call function `func` (saves return address on Stack).
*   `ret`: Finish current function and return to caller.

### Conditionals and Comparisons
*   `cmp a, b`: Compare `a` with `b`.
*   `je` / `jne`: Jump if result is Equal / Not Equal.
*   `test a, b`: Perform bitwise logical AND (usually to check if a value is 0).

### System Instructions
*   `lgdt [ptr]`: Load Global Descriptor Table (GDT) into CPU.
*   `cli` / `sti`: Clear (Disable) / Set (Enable) external interrupts.
*   `hlt`: Halt the CPU (power-saving) until woken by an interrupt.
