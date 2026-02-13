#include "helpers.hpp"
#include "../mm/pmm.hpp"
#include "../drivers/console.hpp"

void meminfo_command() {
    uint64_t total = pmm.get_total_memory();
    uint64_t used = pmm.get_used_memory();
    uint64_t free = pmm.get_free_memory();

    kprint("\n--- Memory Info ---\n");
    kprint("Total Memory: "); kprint_int(total / 1024 / 1024); kprint(" MB ("); kprint_int(total); kprint(" bytes)\n");
    kprint("Used Memory:  "); kprint_int(used / 1024 / 1024); kprint(" MB ("); kprint_int(used); kprint(" bytes)\n");
    kprint("Free Memory:  "); kprint_int(free / 1024 / 1024); kprint(" MB ("); kprint_int(free); kprint(" bytes)\n");
    
    kprint("\n[Leak Test] Allocating 5 frames...\n");
    void* frames[5];
    for(int i=0; i<5; i++) {
        frames[i] = pmm.allocate_frame();
        kprint(" - Alloc: "); kprint_hex((uint64_t)frames[i]); kprint("\n");
    }
    
    kprint("Used after alloc: "); kprint_int(pmm.get_used_memory()); kprint(" bytes\n");
    
    kprint("[Leak Test] Freeing 5 frames...\n");
    for(int i=0; i<5; i++) {
        pmm.free_frame(frames[i]);
    }
    
    kprint("Used after free:  "); kprint_int(pmm.get_used_memory()); kprint(" bytes\n");
    
    if (pmm.get_used_memory() == used) {
        kprint("RESULT: No Leak Detected (Memory returned to original state)\n");
    } else {
        kprint("RESULT: MEMORY LEAK DETECTED!\n");
    }
    kprint("-------------------\n");
}
