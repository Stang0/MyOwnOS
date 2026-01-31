#ifndef PORTS_HPP
#define PORTS_HPP

#include "types.h"

// this function is used to read a byte from a port
static inline uint8_t inb(uint16_t port) {
    uint8_t result;
    asm volatile("inb %1, %0" : "=a"(result) : "Nd"(port));   // read a byte from a port and store it in result in al register
    return result;
}

// this function is used to write a byte to a port
static inline void outb(uint16_t port, uint8_t data) {
    asm volatile("outb %0, %1" : : "a"(data), "Nd"(port)); // write a byte to a port    
}

// this function is used to wait for the I/O to complete
static inline void io_wait() {
    outb(0x80, 0);
}

#endif
