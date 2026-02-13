#include "pmm.hpp"
#include "../arch/x86_64/multiboot.hpp"
#include "../drivers/console.hpp"

// Define static members
uint8_t PhysicalMemoryManager::bitmap[BITMAP_SIZE];
uint64_t PhysicalMemoryManager::total_memory = 0;
uint64_t PhysicalMemoryManager::used_frames = 0;
uint64_t PhysicalMemoryManager::total_frames = 0;

PhysicalMemoryManager pmm;

void PhysicalMemoryManager::init(void* multiboot_info_addr) {
    total_memory = 0;
    total_frames = FRAMES_COUNT;
    used_frames = FRAMES_COUNT;

    // 1. Initialize bitmap: Mark EVERYTHING as used first (safety)
    // We will only free regions that Multiboot tells us are available.
    for (size_t i = 0; i < BITMAP_SIZE; i++) {  // loop all slots in the bitmap (each slot is 8 bytes) 
        bitmap[i] = 0xFF; 
    }
    
    // Cast to uint8_t* for byte arithmetic
    uint8_t* base = (uint8_t*)multiboot_info_addr;
    
    // Skip the first 8 bytes (Total Size + Reserved)
    uint32_t total_size = *(uint32_t*)base;
    uint8_t* tag_ptr = base + 8;
    
    kprint("Parsing Multiboot 2 Information...\n");

    // Loop through tags
    while ((uint32_t)(tag_ptr - base) < total_size) {
        multiboot_tag* tag = (multiboot_tag*)tag_ptr;

        if (tag->type == MULTIBOOT_TAG_TYPE_END) {
            break;
        }

        if (tag->type == MULTIBOOT_TAG_TYPE_MMAP) {
            multiboot_tag_mmap* mmap = (multiboot_tag_mmap*)tag;
            
            kprint("Memory Map Detected:\n");
            
            uint32_t entry_count = (mmap->size - sizeof(multiboot_tag_mmap)) / mmap->entry_size;
            
            for (uint32_t i = 0; i < entry_count; i++) {
                multiboot_mmap_entry* entry = (multiboot_mmap_entry*)((uint8_t*)mmap->entries + (i * mmap->entry_size));
                
                // Only use available memory (Type 1)
                if (entry->type == MULTIBOOT_MEMORY_AVAILABLE) {
                    // Mark this region as free in our bitmap
                    unreserve_region(entry->addr, entry->len);
                    
                    // Update total memory stats (rough estimate)
                    if (entry->addr + entry->len > total_memory) {
                        total_memory = entry->addr + entry->len;
                    }
                }
            }
        }

        // Align tag pointer to 8 bytes
        tag_ptr += (tag->size + 7) & ~7;
    }
    
    // Critical: Mark Kernel memory and Multiboot info as USED!
    // We don't want to allocate over our own code.
    // Assuming Kernel starts at 1MB (0x100000) and ends around 2MB-ish?
    // Let's reserve 0-2MB safely.
    reserve_region(0x0, 0x200000); 
    
    // Also reserve the Multiboot info structure itself
    reserve_region((uint64_t)multiboot_info_addr, total_size);

    if (total_memory == 0 || total_memory > MAX_PHYSICAL_MEMORY) {
        total_memory = MAX_PHYSICAL_MEMORY;
    }
    total_frames = total_memory / PAGE_SIZE;

    // Recount used frames from bitmap to avoid counter drift.
    used_frames = 0;
    for (uint64_t i = 0; i < total_frames; i++) {
        if (!is_frame_free(i)) {
            used_frames++;
        }
    }

    kprint("PMM Initialized.\n");
}

void* PhysicalMemoryManager::allocate_frame() {
    for (uint64_t i = 0; i < total_frames; i++) {
        if (is_frame_free(i)) {
            mark_frame_used(i);
            return (void*)(i * PAGE_SIZE);
        }
    }
    return nullptr; // Out of memory
}

void PhysicalMemoryManager::free_frame(void* ptr) {
    if (!ptr) return;
    uint64_t addr = (uint64_t)ptr;
    uint64_t frame_index = addr / PAGE_SIZE;
    mark_frame_free(frame_index);
}

void PhysicalMemoryManager::mark_frame_used(uint64_t frame_index) {
    if (frame_index >= total_frames) return;
    uint8_t mask = (uint8_t)(1u << (frame_index % 8));
    if (bitmap[frame_index / 8] & mask) return;
    bitmap[frame_index / 8] |= mask;
    used_frames++;
}

void PhysicalMemoryManager::mark_frame_free(uint64_t frame_index) {
    if (frame_index >= total_frames) return;
    uint8_t mask = (uint8_t)(1u << (frame_index % 8));
    if (!(bitmap[frame_index / 8] & mask)) return;
    bitmap[frame_index / 8] &= (uint8_t)~mask;
    used_frames--;
}

bool PhysicalMemoryManager::is_frame_free(uint64_t frame_index) {
    if (frame_index >= total_frames) return false;
    return !(bitmap[frame_index / 8] & (1 << (frame_index % 8)));
}

void PhysicalMemoryManager::reserve_region(uint64_t base, uint64_t length) {
    uint64_t start_frame = base / PAGE_SIZE;
    uint64_t end_frame = (base + length + PAGE_SIZE - 1) / PAGE_SIZE;
    
    for (uint64_t i = start_frame; i < end_frame; i++) {
        if (is_frame_free(i)) {
            mark_frame_used(i);
        }
    }
}

void PhysicalMemoryManager::unreserve_region(uint64_t base, uint64_t length) {
    uint64_t start_frame = base / PAGE_SIZE;
    uint64_t end_frame = (base + length) / PAGE_SIZE; // Round down for safety (don't partial free)
    
    for (uint64_t i = start_frame; i < end_frame; i++) {
        mark_frame_free(i);
    }
}

uint64_t PhysicalMemoryManager::get_total_memory() {
    return total_memory;
}

uint64_t PhysicalMemoryManager::get_used_memory() {
    return used_frames * PAGE_SIZE;
}

uint64_t PhysicalMemoryManager::get_free_memory() {
    return total_memory - get_used_memory();
}
