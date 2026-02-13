#ifndef SERIAL_HPP
#define SERIAL_HPP

#include "../lib/types.h"
#include "../arch/x86_64/ports.hpp"

class Serial {
public:
    static const uint16_t PORT = 0x3F8; // COM1

    static void init() {
        outb(PORT + 1, 0x00);    // Disable all interrupts
        outb(PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
        outb(PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
        outb(PORT + 1, 0x00);    //                  (hi byte)
        outb(PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
        outb(PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
        outb(PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
    }

    static int is_transmit_empty() {
        return inb(PORT + 5) & 0x20;
    }

    static void write_char(char a) {
        while (is_transmit_empty() == 0);
        outb(PORT, a);
    }
    
    static void write_string(const char* str) {
        while (*str) {
            write_char(*str++);
        }
    }
};

#endif
