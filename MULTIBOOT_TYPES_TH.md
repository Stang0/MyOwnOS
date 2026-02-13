# คำอธิบายประเภทข้อมูล Multiboot (Multiboot Types Explanation)

เอกสารนี้รวบรวมความหมายของ **Tag Types** และ **Memory Types** ที่เรากำหนดไว้ในไฟล์ `kernel/arch/x86_64/multiboot.hpp` เพื่อให้เข้าใจว่าแต่ละค่าหมายถึงอะไรครับ

---

## 1. Multiboot Tag Types (ประเภทของป้ายกำกับข้อมูล)

เวลาระบบ Multiboot ส่งข้อมูลให้ Kernel จะส่งมาเป็น "Tag" เหมือนกล่องพัสดุหลายๆ กล่องเรียงต่อกัน ซึ่งแต่ละกล่องจะมีตัวเลขบอกประเภทดังนี้:

| ค่าคงที่ (Define) | ค่าตัวเลข | ความหมาย |
| :--- | :--- | :--- |
| **`MULTIBOOT_TAG_TYPE_END`** | `0` | **จบรายการ:** บอกว่า "หมดแล้ว ไม่มีกล่องต่อไปแล้ว" (เหมือนจุด Full Stop) |
| **`MULTIBOOT_TAG_TYPE_CMDLINE`** | `1` | **Command Line Type:** เก็บข้อความคำสั่งที่ user พิมพ์ต่อท้ายตอนเลือกเมนู Boot (เช่น `kernel.bin debug=on`) |
| **`MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME`** | `2` | **ชื่อ Bootloader:** บอกชื่อโปรแกรมที่โหลดเราขึ้นมา (เช่น "GRUB 2.04") |
| **`MULTIBOOT_TAG_TYPE_MODULE`** | `3` | **Module:** สำหรับ OS ที่โหลดไฟล์เสริมขึ้นมาด้วย (เช่น `initrd.img` หรือ Driver แยก) |
| **`MULTIBOOT_TAG_TYPE_BASIC_MEMINFO`** | `4` | **Basic Memory Info:** บอกขนาดแรมแบบคร่าวๆ (Mem Lower/Upper) แต่ไม่ละเอียดเท่า Memory Map |
| **`MULTIBOOT_TAG_TYPE_BOOTDEV`** | `5` | **Boot Device:** บอกว่า Boot มาจาก Harddisk ลูกไหน หรือ Partition ไหน |
| **`MULTIBOOT_TAG_TYPE_MMAP`** | `6` | **Memory Map (สำคัญที่สุด):** แผนที่แรมฉบับเต็ม บอกละเอียดว่าช่วงไหนว่าง ช่วงไหนห้ามใช้ เราใช้ตัวนี้ทำ PMM |
| **`MULTIBOOT_TAG_TYPE_VBE`** | `7` | **VBE Info:** ข้อมูลเกี่ยวกับกราฟิกการ์ด (กรณีจะทำโหมดกราฟิกภาพสวยๆ) |
| **`MULTIBOOT_TAG_TYPE_FRAMEBUFFER`** | `8` | **Framebuffer:** ข้อมูลหน้าจอที่ถูกตั้งค่าไว้แล้ว (ความกว้าง, สูง, สี) |

---

## 2. Multiboot Memory Types (ประเภทของพื้นที่หน่วยความจำ)

เมื่อเราเปิดดูข้อมูลในกล่อง `MMAP` (Type 6) เราจะเจอรายการพื้นที่ดิน (Memory Region) ซึ่งแต่ละแปลงจะมี "ประเภท" กำกับไว้ดังนี้:

| ค่าคงที่ (Define) | ค่าตัวเลข | ความหมาย | การนำไปใช้ใน OS |
| :--- | :--- | :--- | :--- |
| **`MULTIBOOT_MEMORY_AVAILABLE`** | `1` | **พื้นที่ว่าง (Available):** เป็น RAM เปล่าๆ ที่พร้อมใช้งาน | ✅ **ใช้ได้:** PMM จะเอาส่วนนี้ไปใส่ใน Free List เพื่อแจกจ่ายให้โปรแกรมต่างๆ |
| **`MULTIBOOT_MEMORY_RESERVED`** | `2` | **จองไว้ (Reserved):** ห้ามยุ่ง! อาจเป็นที่อยู่ของ Hardware, ROM หรือ BIOS | ❌ **ห้ามใช้:** ถ้าไปเขียนทับ เครื่องอาจค้างหรือพังได้ |
| **`MULTIBOOT_MEMORY_ACPI_RECLAIMABLE`** | `3` | **ACPI Info:** ข้อมูลการจัดการพลังงาน (ใช้เสร็จแล้วคืนได้ แต่ตอนนี้ยังไม่คืน) | ⚠️ **ระวัง:** OS ขั้นสูงจะอ่านค่านี้เพื่อจัดการ Power Management |
| **`MULTIBOOT_MEMORY_NVS`** | `4` | **NVS (Non-Volatile Storage):** พื้นที่เก็บข้อมูลถาวรตอน Sleep/Hibernate | ❌ **ห้ามใช้:** ต้องเก็บไว้ให้ระบบไฟ |
| **`MULTIBOOT_MEMORY_BADRAM`** | `5` | **RAM เสีย (Bad RAM):** ตรวจพบว่าชิปแรมตรงนี้พัง | ❌ **ห้ามใช้เด็ดขาด:** เขียนแล้วข้อมูลจะเพี้ยน |

---

### สรุปความสำคัญ
ในการเขียน OS ขั้นต้น **เราสนใจแค่ Tag Type 6 (MMAP) และ Memory Type 1 (Available) เท่านั้น** 
เพื่อที่จะรู้ว่าเรามีทรัพยากรให้ใช้เท่าไหร่ ส่วนType อื่นๆ จะเริ่มสำคัญเมื่อเราทำ OS ที่ซับซ้อนขึ้น
