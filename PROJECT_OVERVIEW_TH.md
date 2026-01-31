# คำอธิบายโปรเจกต์ MyOS

นี่คือโปรเจกต์ระบบปฏิบัติการที่เขียนด้วยภาษา Assembly และ C++ ที่ทำเพื่อทำความเข้าใจระบบพื้นฐานของระบบปฏิบัติการ

## โครงสร้างโปรเจกต์ (Project Structure)

ไฟล์ต่างๆ ถูกจัดเก็บ ดังนี้:

### 1. `boot/` (ส่วนการบูตเครื่อง)
*   **`grub.cfg`**: ไฟล์ตั้งค่าของ GRUB bootloader บอกให้ GRUB รู้ว่าจะโหลด Kernel ของเราจากที่ไหนและใช้คำสั่งอะไร

### 2. `kernel/` (ส่วนแกนหลักของระบบ)
ไฟล์หลักที่ควบคุมการทำงานทั้งหมด:
*   **`kernel.cpp`**: จุดเริ่มต้นการทำงานของ OS (Main Entry Point) ที่เป็นภาษา C++ ทำหน้าที่ Initialize ระบบต่างๆ และวนลูปทำงานหลัก

#### `kernel/arch/x86_64/` (ส่วนที่เกี่ยวข้องกับ Hardware โดยตรง)
*   **`boot.asm`**:  
    *   เป็นโค้ด Assembly ด่านแรกที่ทำงานเมื่อเปิดเครื่อง
    *   รับช่วงต่อจาก GRUB (ซึ่งอยู่ในโหมด 32-bit Protected Mode)
    *   **หน้าที่สำคัญ**: ตั้งค่า Page Table, GDT, และ CPU ให้เข้าสู่ **64-bit Long Mode** ก่อนจะกระโดดไปเรียก `kernel_main` ในภาษา C++
*   **`interrupts.cpp` / `.hpp`**: จัดการเรื่อง IDT (Interrupt Descriptor Table) และ PIC (Programmable Interrupt Controller) เพื่อให้ CPU รู้จักการขัดจังหวะจากอุปกรณ์ต่างๆ
*   **`interrupt_stubs.asm`**: โค้ด Assembly เล็กๆ ที่ทำหน้าที่เซฟสถานะของ CPU (Registers) ก่อนจะเรียกฟังก์ชัน C++ เมื่อเกิด Interrupt
*   **`ports.hpp`**: ฟังก์ชันสำหรับคุยกับ Hardware ผ่าน I/O Port (`inb`, `outb`)

#### `kernel/drivers/` (ไดรเวอร์อุปกรณ์ต่างๆ)
*   **`console.cpp` / `.hpp`**:
    *   **VGA Console**: ควบคุมหน้าจอ Text Mode (0xB8000)
    *   รองรับการพิมพ์ตัวอักษร, เปลี่ยนสี, เลื่อนหน้าจอ (Scroll) และขยับ Cursor
*   **`keyboard.cpp` / `.hpp`**:
    *   **Keyboard Driver**: รับค่ากดปุ่มจาก IRQ1 (Port 0x60)
    *   แปลง Scan Code เป็น ASCII
    *   รองรับ **Shift**, **Caps Lock**, **Backspace** และปุ่มตัวเลข/สัญลักษณ์

#### `kernel/lib/` (ไลบรารีพื้นฐาน)
*   **`types.h`**: กำหนดชนิดตัวแปรพื้นฐาน เช่น `uint8_t`, `uint64_t` ให้เป็นมาตรฐานเดียวกับ OS ทั่วไป

### 3. `scripts/` (สคริปต์ช่วย build)
*   **`linker.ld`**: บอก Linker ว่าจะจัดเรียงโค้ดของเราอย่างไรในหน่วยความจำ (เช่น ให้เริ่มที่ Address 1MB และเอา Multiboot Header ไว้หน้าสุด)

### ไฟล์อื่นๆ ที่ Root
*   **`Makefile`**: ชุดคำสั่งสำหรับ Compile และ Build โปรเจกต์ทั้งหมด เพียงพิมพ์ `make` หรือ `make run` ระบบจะจัดการทุกอย่างให้เอง

---

## ขั้นตอนการทำงาน (Flow)

1.  **Boot**: BIOS -> GRUB -> `boot.asm` (ตั้งค่า 64-bit) -> `kernel_main`
2.  **Kernel Init**:
    *   `console.init()`: เตรียมหน้าจอ
    *   `init_interrupts()`: ตั้งค่า IDT/PIC
    *   `irq_install_handler()`: ติดตั้ง Driver คีย์บอร์ด
    *   `sti`: เปิดรับ Interrupt
3.  **Runtime**: OS จะวนลูป (Infinite Loop) รอรับ Interrupt จากคีย์บอร์ด เมื่อมีการกดปุ่ม `keyboard_callback` จะทำงานและแสดงผล

หวังว่าคำอธิบายนี้จะช่วยให้เห็นภาพรวมของโปรเจกต์ชัดเจนขึ้นนะครับ! หากสงสัยจุดไหนถามเพิ่มเติมได้เลย

---

## เครื่องมือที่ใช้ (Tools Used)

โปรเจกต์นี้ใช้เครื่องมือมาตรฐานในการพัฒนา OS ดังนี้:

*   **GCC (g++)**: คอมไพเลอร์สำหรับภาษา C++ ใช้ธงพิเศษอย่าง `-ffreestanding` เพื่อบอกว่าโค้ดนี้ทำงานโดยไม่มี OS (Bare Metal)
*   **NASM**: แอสเซมเบลอร์สำหรับแปลงโค้ด Assembly (.asm) เป็น Machine Code
*   **LD (GNU Linker)**: ตัวเชื่อมโยงไฟล์ Object (.o) ต่างๆ เข้าด้วยกันตามที่กำหนดใน Linker Script (`linker.ld`)
*   **GRUB**: บูตโหลดเดอร์มาตรฐานที่ช่วยโหลด Kernel ของเราขึ้นมาทำงาน
*   **QEMU**: โปรแกรมจำลองเครื่องคอมพิวเตอร์ (Emulator) ช่วยให้เรารันและทดสอบ OS ได้โดยไม่ต้องลงในเครื่องจริง
*   **Make**: เครื่องมือช่วยจัดการกระบวนการ Build (คอมไพล์และลิงก์) โดยอ่านคำสั่งจาก `Makefile`
*   **xorriso**: โปรแกรมสำหรับสร้างไฟล์ ISO image เพื่อนำไปบูต
