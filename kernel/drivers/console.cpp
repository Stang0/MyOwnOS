#include "console.hpp"
#include "ports.hpp"

Console console;

void Console::init() {
    buffer = (volatile uint16_t*)0xB8000;  // address of the video memory 
    row = 0;
    column = 0;
    color = (uint8_t)Color::White | ((uint8_t)Color::Black << 4);
    clear();
}

// Update the hardware cursor to match row/column
void Console::update_cursor() {
    uint16_t pos = row * VGA_WIDTH + column; // calculate the position of the cursor

    outb(0x3D4, 0x0F); // set the cursor position
    outb(0x3D5, (uint8_t)(pos & 0xFF)); // set the cursor position
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void Console::set_color(Color fg, Color bg) {
    color = (uint8_t)fg | ((uint8_t)bg << 4);
}

void Console::put_entry_at(char c, uint8_t color, size_t x, size_t y) {
    const size_t index = y * VGA_WIDTH + x; // calculate the index of the entry
    buffer[index] = (uint16_t)c | ((uint16_t)color << 8); // set the entry
}

void Console::clear() {
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            put_entry_at(' ', color, x, y);  // clear the screen with loop 
        }
    }
    row = 0;
    column = 0;
    update_cursor();
}

void Console::scroll() {
    // Move everything up by one row
    for (size_t y = 0; y < VGA_HEIGHT - 1; y++) { // loop through the rows
        for (size_t x = 0; x < VGA_WIDTH; x++) { // loop through the columns
            const size_t src_index = (y + 1) * VGA_WIDTH + x; // calculate the source index
            const size_t dst_index = y * VGA_WIDTH + x; // calculate the destination index
            buffer[dst_index] = buffer[src_index]; // copy the source to the destination
        }
    }

    // Clear the last row
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        put_entry_at(' ', color, x, VGA_HEIGHT - 1);
    }
}

void Console::write_char(char c) {
    if (c == '\n') {
        column = 0;
        if (++row == VGA_HEIGHT) { // if the row is equal to the height of the screen
            scroll();
            row = VGA_HEIGHT - 1;
        }
    } else if (c == '\r') {
        column = 0; // reset the column
    } else if (c == '\b') { // if the character is a backspace
        if (column > 0) {
            column--;
            put_entry_at(' ', color, column, row);
        } else if (row > 0) {
            row--;
            column = VGA_WIDTH - 1;
            put_entry_at(' ', color, column, row);
        }
    } else {
        put_entry_at(c, color, column, row); // put the character in the buffer
        if (++column == VGA_WIDTH) { // if the column is equal to the width of the screen
            column = 0;
            if (++row == VGA_HEIGHT) { // if the row is equal to the height of the screen
                scroll();
                row = VGA_HEIGHT - 1;
            }
        }
    }
    update_cursor();
}

void Console::write_string(const char* str) {
    while (*str) {
        write_char(*str++);
    }
}

void Console::write_string(const char* str, Color fg) {
    uint8_t old_color = color;
    set_color(fg, (Color)(old_color >> 4));
    write_string(str);
    color = old_color;
}

void Console::panic_screen(const char* msg) {
    set_color(Color::White, Color::Red); // set the color to white on red
    clear();
    write_string("!!! KERNEL PANIC !!!\n\n");
    write_string(msg);
    write_string("\n\nSystem Halted.");
}

// Global Implementations

void kprint(const char* str) {
    console.write_string(str);
}

void klog(const char* msg) {
    console.write_string("[LOG] ");
    console.write_string(msg);
    console.write_char('\n');
}

void panic(const char* msg) {
    console.panic_screen(msg);
    while (1) {
        asm volatile("hlt");
    }
}
