#include "drivers/console.hpp"
#include "arch/x86_64/interrupts.hpp"
#include "drivers/keyboard.hpp"
#include "mm/pmm.hpp"
#include "drivers/serial.hpp"
#include "lib/helpers.hpp"
#include "fs/tarfs.hpp"

extern "C" uint8_t _binary_build_initrd_tar_start[];
extern "C" uint8_t _binary_build_initrd_tar_end[];

extern "C" void kernel_main(void* multiboot_info) {
    // Initialize the console driver
    console.init();
    Serial::init();
    
    // Print welcome messages 
    kprint("=== MyOS Kernel v0.1 ===\n");
    console.set_color(Color::LightBlue, Color::Black);
    kprint("Platform: x86_64 Long Mode\n");
    
    // Initialize Memory Manager
    pmm.init(multiboot_info);
    
    // Test Allocation
    void* p1 = pmm.allocate_frame();
    void* p2 = pmm.allocate_frame();
    void* p3 = pmm.allocate_frame();
    
    kprint("Allocated Frames at: ");
    kprint_hex((uint64_t)p1); kprint(", ");
    kprint_hex((uint64_t)p2); kprint(", ");
    kprint_hex((uint64_t)p3); kprint("\n");
    
    
    pmm.free_frame(p2);
    kprint("Freed frame 2\n");
    
    void* p4 = pmm.allocate_frame();
    kprint("Allocated new frame at: ");
    kprint_hex((uint64_t)p4); kprint("\n");

    // Auto-run meminfo for verification
    meminfo_command();

    size_t initrd_size = (size_t)(_binary_build_initrd_tar_end - _binary_build_initrd_tar_start);
    tarfs_init(_binary_build_initrd_tar_start, initrd_size);
    tarfs_self_test();

    console.set_color(Color::LightGray, Color::Black);
    kprint("[Press Ctrl+A X to exit QEMU]\n\n");

    // Initialize Interrupts and Keyboard
    klog("Initializing Interrupts...");
    init_interrupts();
    
    klog("Initializing Keyboard...");
    init_keyboard();  //IRQ 1 init   

    klog("Enabling Interrupts...");
    asm volatile("sti"); // enable interrupts for keyboard (IRQ 1)
    
    klog("System Ready. Commands: meminfo, ls, cat <file>");
    kprint("> ");

    while (1) {
        asm volatile("hlt"); // for power saving by stop CPU Execution
    }
}
