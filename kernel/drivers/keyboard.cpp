#include "keyboard.hpp"
#include "interrupts.hpp"
#include "ports.hpp"
#include "console.hpp"

// Simple US QWERTY Scan Code Set 1 Map
unsigned char kbdus[128] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
  '9', '0', '-', '=', '\b',	/* Backspace */
  '\t',			/* Tab */
  'q', 'w', 'e', 'r',	/* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
    0,			/* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
 '\'', '`',   0,		/* Left shift */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0,				/* Right shift */
  '*',
    0,	/* Alt */
  ' ',	/* Space bar */
    0,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   0,
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
};

unsigned char kbdus_shift[128] =
{
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*',	/* 9 */
  '(', ')', '_', '+', '\b',	/* Backspace */
  '\t',			/* Tab */
  'Q', 'W', 'E', 'R',	/* 19 */
  'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',	/* Enter key */
    0,			/* 29   - Control */
  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',	/* 39 */
 '"', '~',   0,		/* Left shift */
 '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?',   0,				/* Right shift */
  '*',
    0,	/* Alt */
  ' ',	/* Space bar */
    0,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   0,
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
};

static bool is_shift = false;
static bool is_caps_lock = false;

void keyboard_callback(Registers* regs) {
    uint8_t scancode = inb(0x60); //read  scan code from port  0x60 
    
    // Handle specific keys before checking for release
    if (scancode == 0x2A || scancode == 0x36) { // Left or Right Shift Down
        is_shift = true;
        return;
    } else if (scancode == 0xAA || scancode == 0xB6) { // Left or Right Shift Up
        is_shift = false;
        return;
    } else if (scancode == 0x3A) { // Caps Lock Down
        is_caps_lock = !is_caps_lock;
        return;
    }

    // Ignore other break codes
    if (scancode & 0x80) {
        return;
    }

    // Key pressed (Make code)
    char c = 0;
    
    // Choose which map to use
    if (is_shift) {
        c = kbdus_shift[scancode];
    } else {
        c = kbdus[scancode];
    }

    if (c != 0) {
        // Handle Caps Lock for letters
        if (is_caps_lock) {
            if (c >= 'a' && c <= 'z') {
                c -= 32; // To Upper
            } else if (c >= 'A' && c <= 'Z') {
                c += 32; // To Lower
            }
        }
        
        console.write_char(c); // print the character to the console
    }
    
    (void)regs; // Unused
}

void init_keyboard() {
    irq_install_handler(1, keyboard_callback);
}
