#ifndef CONSOLE_HPP
#define CONSOLE_HPP

#include "types.h"

enum class Color : uint8_t {
    Black = 0,
    Blue = 1,
    Green = 2,
    Cyan = 3,
    Red = 4,
    Magenta = 5,
    Brown = 6,
    LightGray = 7,
    DarkGray = 8,
    LightBlue = 9,
    LightGreen = 10,
    LightCyan = 11,
    LightRed = 12,
    Pink = 13,
    Yellow = 14,
    White = 15
};

class Console {
public:
    static const size_t VGA_WIDTH = 80;
    static const size_t VGA_HEIGHT = 25;

    void init();
    void clear();
    void write_char(char c);
    void write_string(const char* str);
    void write_string(const char* str, Color color);
    void set_color(Color fg, Color bg);
    void panic_screen(const char* msg);

private:
    void scroll();
    void put_entry_at(char c, uint8_t color, size_t x, size_t y);
    void update_cursor();

    size_t row;
    size_t column;
    uint8_t color;
    volatile uint16_t* buffer;
};

extern Console console;

// Global helper functions
void kprint(const char* str);
void klog(const char* msg);
void panic(const char* msg);

#endif
