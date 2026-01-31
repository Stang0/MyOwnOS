#include "drivers/console.hpp"
#include "arch/x86_64/interrupts.hpp"
#include "drivers/keyboard.hpp"

extern "C" void kernel_main(void* multiboot_info) {
    // Initialize the console driver
    console.init();
    
    // Print welcome messages 
    kprint("=== MyOS Kernel v0.1 ===\n");
    console.set_color(Color::LightBlue, Color::Black);
    kprint("Platform: x86_64 Long Mode\n");
    
    console.set_color(Color::LightGreen, Color::Black);
    kprint("Week 1: Boot + VGA Console Complete!\n");

    console.set_color(Color::LightGray, Color::Black);
    kprint("[Press Ctrl+A X to exit QEMU]\n\n");

    // Initialize Interrupts and Keyboard
    klog("Initializing Interrupts...");
    init_interrupts();
    
    klog("Initializing Keyboard...");
    init_keyboard();  //IRQ 1 init   

    klog("Enabling Interrupts...");
    asm volatile("sti"); // enable interrupts for keyboard (IRQ 1)
    
    klog("System Ready. Type something!");

    while (1) {
        asm volatile("hlt"); // for power saving by stop CPU Execution
    }
}
