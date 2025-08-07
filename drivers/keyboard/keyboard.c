#include "../keyboard/keyboard.h"
#include "../ports/ports.h"
#include "../../cpu/isr.h"
#include "../display/display.h"
#include "../../apps/shell.h"
#include "../../kernel/util.h"
#include "keys.h"
#include <stdbool.h>

static char key_buffer[256];
bool is_shift_down = false;

// const char scancode_to_char[] = {
//     '?', '`', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',
//     '?', '?', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']',
//     '?', '?', 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '#',
//    '?', '\\', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '?',
//                 '?', '?', ' '
// };

const char scancode_to_char_lower[] = {
  '￼', '`', '1', '2', '3', '4', '5',
  '6', '7', '8', '9', '0', '-', '=',
  '￼', '￼', 'q', 'w', 'e', 'r', 't',
  'y', 'u', 'i', 'o', 'p', '[', ']',
  '￼', '￼', 'a', 's', 'd', 'f', 'g',
  'h', 'j', 'k', 'l', ';', '\'', '#',
  '￼', '\\', 'z', 'x', 'c', 'v', 'b',
  'n', 'm', ',', '.', '/', '￼', '￼',
  '￼', ' '
};

const char scancode_to_char_upper[] = {
  '￼', '¬', '!', '"', '£', '$', '%',
  '^', '&', '*', '(', ')', '_', '+',
  '￼', '￼', 'Q', 'W', 'E', 'R', 'T',
  'Y', 'U', 'I', 'O', 'P', '{', '}',
  '￼', '￼', 'A', 'S', 'D', 'F', 'G',
  'H', 'J', 'K', 'L', ':', '@', '~',
  '￼', '|', 'Z', 'X', 'C', 'V', 'B',
  'N', 'M', '<', '>', '?', '￼', '￼',
  '￼', ' '
};

static void keyboard_callback(registers_t *regs) {
    uint8_t scancode = port_byte_in(0x60);

    keycode_to_char(scancode);
    keys_debug(scancode);

    if (scancode > SC_MAX) return;

    int blocked_keys[] = { 0x00, 0x01, 0x0F, 0x1D, 0x29, 0x36, 0x38, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x57, 0x58, 0x5D };

    int i;
    for (i = 0; i < sizeof(blocked_keys)/sizeof(int); i++) {
        if (blocked_keys[i] == scancode) return '\0';
    }

    if (scancode == BACKSPACE) {
        if (backspace(key_buffer)) {
            print_backspace();
        }
    } else if (scancode == ENTER) {
        print_nl();
        execute_command(key_buffer);
        key_buffer[0] = '\0';
    } else {
        char letter;
        letter = keycode_to_char(scancode);
        append(key_buffer, letter);
        char str[2] = {letter, '\0'};
        print_string(str);
    }
}

void init_keyboard() {
    // Flush any leftover scancodes from the buffer
    while (port_byte_in(0x64) & 0x01) {
        port_byte_in(0x60);
    }

    register_interrupt_handler(IRQ1, keyboard_callback);
}