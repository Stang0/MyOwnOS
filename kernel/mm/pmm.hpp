#ifndef PMM_HPP
#define PMM_HPP

#include "../lib/types.h"

// 4KB Page Size
#define PAGE_SIZE 4096

// Max supported memory size (e.g., 128MB for now, can be increased)
// 128MB = 32768 pages
// Bitmap size = 32768 / 8 = 4096 bytes (4KB)
#define MAX_PHYSICAL_MEMORY (128 * 1024 * 1024)
#define FRAMES_COUNT (MAX_PHYSICAL_MEMORY / PAGE_SIZE)
#define BITMAP_SIZE (FRAMES_COUNT / 8)

class PhysicalMemoryManager {
public:
    static void init(void* multiboot_info_addr); // Initialize PMM with Multiboot info
    static void* allocate_frame(); // Allocate a free frame
    static void free_frame(void* ptr); // Free a frame
    
    // Debug info
    static uint64_t get_total_memory(); // Get total memory size
    static uint64_t get_free_memory(); // Get free memory size
    static uint64_t get_used_memory(); // Get used memory size

private:
    static void mark_frame_used(uint64_t frame_index); // Mark a frame as used
    static void mark_frame_free(uint64_t frame_index); // Mark a frame as free
    static bool is_frame_free(uint64_t frame_index);
    static void reserve_region(uint64_t base, uint64_t length);
    static void unreserve_region(uint64_t base, uint64_t length);

    // The bitmap: 1 bit per page. 0 = free, 1 = used.
    static uint8_t bitmap[BITMAP_SIZE];
    static uint64_t total_memory;
    static uint64_t used_frames;
    static uint64_t total_frames;
};

extern PhysicalMemoryManager pmm;

#endif
