#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "keys.h"
#include "../display/display.h"
#include "../../kernel/util.h"
#include "../../kernel/mem.h"

#define RELEASE_OFFSET 0x80

int FILTER = 0b00000000;

void keys_debug(int keycode) {
    char* str8 = mem_alloc(8 * sizeof(char));
    bool debug_filter = true;
    bool debug_keycode = true;

    if (debug_filter == true) {
        char* str = dec_to_bin(FILTER, 8);

        int i;
        for (i = 0; i < 8; i++) {
            set_char_at_video_memory_color(str[i], get_offset(0 + i, 0), 0x27);
        }
    }

    if (debug_keycode == true) {
        char pressed_char[4];
        if (keycode >= 0x7F) {
            int_to_string(keycode, pressed_char - 0x80);
        } else {
            int_to_string(keycode, pressed_char);
        }
        int i;
        for (i = 0; i < 4; i++) {
            set_char_at_video_memory_color(pressed_char[i], get_offset(0 + i, 1), 0x27);
        }
    }
}

char keycode_to_char(int keycode) {

    #pragma region MODIFIERS
    // SHIFT HOLD
    if (keycode == L_SHIFT || keycode == R_SHIFT) {
        FILTER |= SHIFT_HELD;
        return '\0';
    } if (keycode == L_SHIFT + RELEASE_OFFSET || keycode == R_SHIFT + RELEASE_OFFSET) {
        FILTER &= ~SHIFT_HELD;
        return '\0';
    }

    // CTRL HOLD
    if (keycode == CTRL) {
        FILTER |= CTRL_HELD;
        return '\0';
    } if (keycode == CTRL + RELEASE_OFFSET) {
        FILTER &= ~CTRL_HELD;
        return '\0';
    }

    // ALT HOLD
    if (keycode == ALT) {
        FILTER |= ALT_HELD;
        return '\0';
    } if (keycode == ALT + RELEASE_OFFSET) {
        FILTER &= ~ALT_HELD;
        return '\0';
    }

    // MOD HOLD
    if (keycode == MOD) {
        FILTER |= MOD_HELD;
        return '\0';
    } if (keycode == MOD + RELEASE_OFFSET) {
        FILTER &= ~MOD_HELD;
        return '\0';
    }

    // CAPSLOCK
    if (keycode == CAPSLOCK) {
        FILTER ^= CAPSLOCK_TOG;
        return '\0';
    }

    // NUMLOCK
    if (keycode == NUMLOCK) {
        FILTER ^= NUM_LOCK_TOG;
        return '\0';
    }

    // MODLOCK
    if (keycode == MODLOCK) {
        FILTER ^= MOD_LOCK_TOG;
        return '\0';
    }
    #pragma endregion
    
    // SHIFT + CAPS LOCK
    bool s_state    = (((FILTER & SHIFT_HELD)   == SHIFT_HELD)^((FILTER & CAPSLOCK_TOG) == CAPSLOCK_TOG));
    // CTRL
    bool c_state    = (FILTER & CTRL_HELD)      == CTRL_HELD;
    // ALT
    bool a_state    = (FILTER & ALT_HELD)       == ALT_HELD;
    // MOD + MOD LOCK
    bool m_state    = (((FILTER & MOD_HELD)     == MOD_HELD)^((FILTER & MOD_LOCK_TOG) == MOD_LOCK_TOG));
    // NUM LOCK
    bool n_state    = (FILTER & NUM_LOCK_TOG)   == NUM_LOCK_TOG;

    // Recompress the filter
    int compressed_filter = (s_state * SHIFT_HELD) + 
                            (c_state * CTRL_HELD) + 
                            (a_state * ALT_HELD) + 
                            (m_state * MOD_HELD);

    // Deal with numpad first
    if (keycode >= 0x47 && 0x53 >= keycode) {
        switch (keycode)
        {
            case 0x4A: return '-';
            case 0x4E: return '+';
            case 0x53: return '.';
        }

        if (n_state) {
            switch (keycode)
            {
                case 0x47: return '7';
                case 0x48: return '8';
                case 0x49: return '9';
                case 0x4B: return '4';
                case 0x4C: return '5';
                case 0x4D: return '6';
                case 0x4F: return '1';
                case 0x50: return '2';
                case 0x51: return '3';
                case 0x52: return '0';
            }
        }
        else { return '\0'; }
    }

    else {
        switch (compressed_filter) {
                case 0b0000: // NO MODIFIERS
                    switch (keycode) {
                        case 0x02: return '1';
                        case 0x03: return '2';
                        case 0x04: return '3';
                        case 0x05: return '4';
                        case 0x06: return '5';
                        case 0x07: return '6';
                        case 0x08: return '7';
                        case 0x09: return '8';
                        case 0x0A: return '9';
                        case 0x0B: return '0';
                        case 0x0C: return '-';
                        case 0x0D: return '=';
                        case 0x10: return 'q';
                        case 0x11: return 'w';
                        case 0x12: return 'e';
                        case 0x13: return 'r';
                        case 0x14: return 't';
                        case 0x15: return 'y';
                        case 0x16: return 'u';
                        case 0x17: return 'i';
                        case 0x18: return 'o';
                        case 0x19: return 'p';
                        case 0x1A: return '[';
                        case 0x1B: return ']';
                        case 0x1E: return 'a';
                        case 0x1F: return 's';
                        case 0x20: return 'd';
                        case 0x21: return 'f';
                        case 0x22: return 'g';
                        case 0x23: return 'h';
                        case 0x24: return 'j';
                        case 0x25: return 'k';
                        case 0x26: return 'l';
                        case 0x27: return ';';
                        case 0x28: return '\'';
                        case 0x2A: return '`';
                        case 0x2B: return '#';
                        case 0x2C: return 'z';
                        case 0x2D: return 'x';
                        case 0x2E: return 'c';
                        case 0x2F: return 'v';
                        case 0x30: return 'b';
                        case 0x31: return 'n';
                        case 0x32: return 'm';
                        case 0x33: return ',';
                        case 0x34: return '.';
                        case 0x35: return '/';
                        case 0x37: return '*';
                        case 0x39: return ' ';
                    }
                case 0b0001: // SHIFT
                    switch (keycode) {
                        case 0x02: return '!';
                        case 0x03: return '"';
                        case 0x04: return '£';
                        case 0x05: return '$';
                        case 0x06: return '%';
                        case 0x07: return '^';
                        case 0x08: return '&';
                        case 0x09: return '*';
                        case 0x0A: return '(';
                        case 0x0B: return ')';
                        case 0x0C: return '_';
                        case 0x0D: return '+';
                        case 0x10: return 'Q';
                        case 0x11: return 'W';
                        case 0x12: return 'E';
                        case 0x13: return 'R';
                        case 0x14: return 'T';
                        case 0x15: return 'Y';
                        case 0x16: return 'U';
                        case 0x17: return 'I';
                        case 0x18: return 'O';
                        case 0x19: return 'P';
                        case 0x1A: return '{';
                        case 0x1B: return '}';
                        case 0x1E: return 'A';
                        case 0x1F: return 'S';
                        case 0x20: return 'D';
                        case 0x21: return 'F';
                        case 0x22: return 'G';
                        case 0x23: return 'H';
                        case 0x24: return 'J';
                        case 0x25: return 'K';
                        case 0x26: return 'L';
                        case 0x27: return ':';
                        case 0x28: return '@';
                        case 0x2A: return '¬';
                        case 0x2B: return '~';
                        case 0x2C: return 'Z';
                        case 0x2D: return 'X';
                        case 0x2E: return 'C';
                        case 0x2F: return 'V';
                        case 0x30: return 'B';
                        case 0x31: return 'N';
                        case 0x32: return 'M';
                        case 0x33: return '<';
                        case 0x34: return '>';
                        case 0x35: return '?';
                        case 0x37: return '*';
                        case 0x39: return ' ';
                    }
                case 0b0010: // CTRL
                    return '\0';
                case 0b0011: // CTRL + SHIFT
                    return '\0';
                case 0b0100: // ALT
                    switch (keycode) {
                        case 0x02: return '1';
                        case 0x03: return '2';
                        case 0x04: return '3';
                        case 0x05: return '4';
                        case 0x06: return '5';
                        case 0x07: return '6';
                        case 0x08: return '7';
                        case 0x09: return '8';
                        case 0x0A: return '9';
                        case 0x0B: return '0';
                        case 0x0C: return '-';
                        case 0x0D: return '=';
                        case 0x10: return 'q';
                        case 0x11: return 'w';
                        case 0x12: return 'e';
                        case 0x13: return 'r';
                        case 0x14: return 't';
                        case 0x15: return 'y';
                        case 0x16: return 'u';
                        case 0x17: return 'i';
                        case 0x18: return 'o';
                        case 0x19: return 'p';
                        case 0x1A: return '[';
                        case 0x1B: return ']';
                        case 0x1E: return 'a';
                        case 0x1F: return 's';
                        case 0x20: return 'd';
                        case 0x21: return 'f';
                        case 0x22: return 'g';
                        case 0x23: return 'h';
                        case 0x24: return 'j';
                        case 0x25: return 'k';
                        case 0x26: return 'l';
                        case 0x27: return ';';
                        case 0x28: return '\'';
                        case 0x2A: return '`';
                        case 0x2B: return '#';
                        case 0x2C: return 'z';
                        case 0x2D: return 'x';
                        case 0x2E: return 'c';
                        case 0x2F: return 'v';
                        case 0x30: return 'b';
                        case 0x31: return 'n';
                        case 0x32: return 'm';
                        case 0x33: return ',';
                        case 0x34: return '.';
                        case 0x35: return '/';
                        case 0x37: return '*';
                        case 0x39: return ' ';
                    }
                case 0b0101: // ALT + SHIFT
                    switch (keycode) {
                        case 0x02: return '!';
                        case 0x03: return '"';
                        case 0x04: return '£';
                        case 0x05: return '$';
                        case 0x06: return '%';
                        case 0x07: return '^';
                        case 0x08: return '&';
                        case 0x09: return '*';
                        case 0x0A: return '(';
                        case 0x0B: return ')';
                        case 0x0C: return '_';
                        case 0x0D: return '+';
                        case 0x10: return 'Q';
                        case 0x11: return 'W';
                        case 0x12: return 'E';
                        case 0x13: return 'R';
                        case 0x14: return 'T';
                        case 0x15: return 'Y';
                        case 0x16: return 'U';
                        case 0x17: return 'I';
                        case 0x18: return 'O';
                        case 0x19: return 'P';
                        case 0x1A: return '{';
                        case 0x1B: return '}';
                        case 0x1E: return 'A';
                        case 0x1F: return 'S';
                        case 0x20: return 'D';
                        case 0x21: return 'F';
                        case 0x22: return 'G';
                        case 0x23: return 'H';
                        case 0x24: return 'J';
                        case 0x25: return 'K';
                        case 0x26: return 'L';
                        case 0x27: return ':';
                        case 0x28: return '@';
                        case 0x2A: return '¬';
                        case 0x2B: return '~';
                        case 0x2C: return 'Z';
                        case 0x2D: return 'X';
                        case 0x2E: return 'C';
                        case 0x2F: return 'V';
                        case 0x30: return 'B';
                        case 0x31: return 'N';
                        case 0x32: return 'M';
                        case 0x33: return '<';
                        case 0x34: return '>';
                        case 0x35: return '?';
                        case 0x37: return '*';
                        case 0x39: return ' ';
                    }
                case 0b0110: // ALT + CTRL
                    switch (keycode) {
                        case 0x02: return '1';
                        case 0x03: return '2';
                        case 0x04: return '3';
                        case 0x05: return '€';
                        case 0x06: return '5';
                        case 0x07: return '6';
                        case 0x08: return '7';
                        case 0x09: return '8';
                        case 0x0A: return '9';
                        case 0x0B: return '0';
                        case 0x0C: return '-';
                        case 0x0D: return '=';
                        case 0x10: return 'q';
                        case 0x11: return 'w';
                        case 0x12: return 'é';
                        case 0x13: return 'r';
                        case 0x14: return 't';
                        case 0x15: return 'y';
                        case 0x16: return 'ú';
                        case 0x17: return 'í';
                        case 0x18: return 'ó';
                        case 0x19: return 'p';
                        case 0x1A: return '[';
                        case 0x1B: return ']';
                        case 0x1E: return 'á';
                        case 0x1F: return 's';
                        case 0x20: return 'd';
                        case 0x21: return 'f';
                        case 0x22: return 'g';
                        case 0x23: return 'h';
                        case 0x24: return 'j';
                        case 0x25: return 'k';
                        case 0x26: return 'l';
                        case 0x27: return ';';
                        case 0x28: return '\'';
                        case 0x2A: return '¦';
                        case 0x2B: return '#';
                        case 0x2C: return 'z';
                        case 0x2D: return 'x';
                        case 0x2E: return 'c';
                        case 0x2F: return 'v';
                        case 0x30: return 'b';
                        case 0x31: return 'n';
                        case 0x32: return 'm';
                        case 0x33: return ',';
                        case 0x34: return '.';
                        case 0x35: return '/';
                        case 0x37: return '*';
                        case 0x39: return ' ';
                    }
                case 0b0111: // ALT + CTRL + SHIFT
                    switch (keycode) {
                        case 0x02: return '!';
                        case 0x03: return '"';
                        case 0x04: return '£';
                        case 0x05: return '$';
                        case 0x06: return '%';
                        case 0x07: return '^';
                        case 0x08: return '&';
                        case 0x09: return '*';
                        case 0x0A: return '(';
                        case 0x0B: return ')';
                        case 0x0C: return '_';
                        case 0x0D: return '+';
                        case 0x10: return 'Q';
                        case 0x11: return 'W';
                        case 0x12: return 'É';
                        case 0x13: return 'R';
                        case 0x14: return 'T';
                        case 0x15: return 'Y';
                        case 0x16: return 'Ú';
                        case 0x17: return 'Í';
                        case 0x18: return 'Ó';
                        case 0x19: return 'P';
                        case 0x1A: return '{';
                        case 0x1B: return '}';
                        case 0x1E: return 'Á';
                        case 0x1F: return 'S';
                        case 0x20: return 'D';
                        case 0x21: return 'F';
                        case 0x22: return 'G';
                        case 0x23: return 'H';
                        case 0x24: return 'J';
                        case 0x25: return 'K';
                        case 0x26: return 'L';
                        case 0x27: return ':';
                        case 0x28: return '@';
                        case 0x2A: return '¬';
                        case 0x2B: return '~';
                        case 0x2C: return 'Z';
                        case 0x2D: return 'X';
                        case 0x2E: return 'C';
                        case 0x2F: return 'V';
                        case 0x30: return 'B';
                        case 0x31: return 'N';
                        case 0x32: return 'M';
                        case 0x33: return '<';
                        case 0x34: return '>';
                        case 0x35: return '?';
                        case 0x37: return '*';
                        case 0x39: return ' ';
                    }
                case 0b1000: // MOD
                    switch (keycode) {
                        case 0x02: return '1';
                        case 0x03: return '2';
                        case 0x04: return '3';
                        case 0x05: return '4';
                        case 0x06: return '5';
                        case 0x07: return '6';
                        case 0x08: return '7';
                        case 0x09: return '8';
                        case 0x0A: return '9';
                        case 0x0B: return '0';
                        case 0x0C: return '-';
                        case 0x0D: return '=';
                        case 0x10: return 'q';
                        case 0x11: return 'w';
                        case 0x12: return 'e';
                        case 0x13: return 'r';
                        case 0x14: return 't';
                        case 0x15: return 'y';
                        case 0x16: return 'u';
                        case 0x17: return 'i';
                        case 0x18: return 'o';
                        case 0x19: return 'p';
                        case 0x1A: return '[';
                        case 0x1B: return ']';
                        case 0x1E: return 'a';
                        case 0x1F: return 's';
                        case 0x20: return 'd';
                        case 0x21: return 'f';
                        case 0x22: return 'g';
                        case 0x23: return 'h';
                        case 0x24: return 'j';
                        case 0x25: return 'k';
                        case 0x26: return 'l';
                        case 0x27: return ';';
                        case 0x28: return '\'';
                        case 0x2A: return '`';
                        case 0x2B: return '#';
                        case 0x2C: return 'z';
                        case 0x2D: return 'x';
                        case 0x2E: return 'c';
                        case 0x2F: return 'v';
                        case 0x30: return 'b';
                        case 0x31: return 'n';
                        case 0x32: return 'm';
                        case 0x33: return ',';
                        case 0x34: return '.';
                        case 0x35: return '/';
                        case 0x37: return '*';
                        case 0x39: return ' ';
                    }
                case 0b1001: // MOD + SHIFT
                    switch (keycode) {
                        case 0x02: return '!';
                        case 0x03: return '"';
                        case 0x04: return '£';
                        case 0x05: return '$';
                        case 0x06: return '%';
                        case 0x07: return '^';
                        case 0x08: return '&';
                        case 0x09: return '*';
                        case 0x0A: return '(';
                        case 0x0B: return ')';
                        case 0x0C: return '_';
                        case 0x0D: return '+';
                        case 0x10: return 'Q';
                        case 0x11: return 'W';
                        case 0x12: return 'E';
                        case 0x13: return 'R';
                        case 0x14: return 'T';
                        case 0x15: return 'Y';
                        case 0x16: return 'U';
                        case 0x17: return 'I';
                        case 0x18: return 'O';
                        case 0x19: return 'P';
                        case 0x1A: return '{';
                        case 0x1B: return '}';
                        case 0x1E: return 'A';
                        case 0x1F: return 'S';
                        case 0x20: return 'D';
                        case 0x21: return 'F';
                        case 0x22: return 'G';
                        case 0x23: return 'H';
                        case 0x24: return 'J';
                        case 0x25: return 'K';
                        case 0x26: return 'L';
                        case 0x27: return ':';
                        case 0x28: return '@';
                        case 0x2A: return '¬';
                        case 0x2B: return '~';
                        case 0x2C: return 'Z';
                        case 0x2D: return 'X';
                        case 0x2E: return 'C';
                        case 0x2F: return 'V';
                        case 0x30: return 'B';
                        case 0x31: return 'N';
                        case 0x32: return 'M';
                        case 0x33: return '<';
                        case 0x34: return '>';
                        case 0x35: return '?';
                        case 0x37: return '*';
                        case 0x39: return ' ';
                        
                    }
                case 0b1010: // MOD + CTRL
                    return '\0';
                case 0b1011: // MOD + CTRL + SHIFT
                    return '\0';
                case 0b1100: // MOD + ALT
                    switch (keycode) {
                        case 0x02: return '1';
                        case 0x03: return '2';
                        case 0x04: return '3';
                        case 0x05: return '4';
                        case 0x06: return '5';
                        case 0x07: return '6';
                        case 0x08: return '7';
                        case 0x09: return '8';
                        case 0x0A: return '9';
                        case 0x0B: return '0';
                        case 0x0C: return '-';
                        case 0x0D: return '=';
                        case 0x10: return 'q';
                        case 0x11: return 'w';
                        case 0x12: return 'e';
                        case 0x13: return 'r';
                        case 0x14: return 't';
                        case 0x15: return 'y';
                        case 0x16: return 'u';
                        case 0x17: return 'i';
                        case 0x18: return 'o';
                        case 0x19: return 'p';
                        case 0x1A: return '[';
                        case 0x1B: return ']';
                        case 0x1E: return 'a';
                        case 0x1F: return 's';
                        case 0x20: return 'd';
                        case 0x21: return 'f';
                        case 0x22: return 'g';
                        case 0x23: return 'h';
                        case 0x24: return 'j';
                        case 0x25: return 'k';
                        case 0x26: return 'l';
                        case 0x27: return ';';
                        case 0x28: return '\'';
                        case 0x2A: return '`';
                        case 0x2B: return '#';
                        case 0x2C: return 'z';
                        case 0x2D: return 'x';
                        case 0x2E: return 'c';
                        case 0x2F: return 'v';
                        case 0x30: return 'b';
                        case 0x31: return 'n';
                        case 0x32: return 'm';
                        case 0x33: return ',';
                        case 0x34: return '.';
                        case 0x35: return '/';
                        case 0x37: return '*';
                        case 0x39: return ' ';
                        
                    }
                case 0b1101: // MOD + ALT + SHIFT
                    switch (keycode) {
                        case 0x02: '!';
                        case 0x03: '"';
                        case 0x04: '£';
                        case 0x05: '$';
                        case 0x06: '%';
                        case 0x07: '^';
                        case 0x08: '&';
                        case 0x09: '*';
                        case 0x0A: '(';
                        case 0x0B: ')';
                        case 0x0C: '_';
                        case 0x0D: '+';
                        case 0x10: 'Q';
                        case 0x11: 'W';
                        case 0x12: 'E';
                        case 0x13: 'R';
                        case 0x14: 'T';
                        case 0x15: 'Y';
                        case 0x16: 'U';
                        case 0x17: 'I';
                        case 0x18: 'O';
                        case 0x19: 'P';
                        case 0x1A: '{';
                        case 0x1B: '}';
                        case 0x1E: 'A';
                        case 0x1F: 'S';
                        case 0x20: 'D';
                        case 0x21: 'F';
                        case 0x22: 'G';
                        case 0x23: 'H';
                        case 0x24: 'J';
                        case 0x25: 'K';
                        case 0x26: 'L';
                        case 0x27: ':';
                        case 0x28: '@';
                        case 0x2A: '¬';
                        case 0x2B: '~';
                        case 0x2C: 'Z';
                        case 0x2D: 'X';
                        case 0x2E: 'C';
                        case 0x2F: 'V';
                        case 0x30: 'B';
                        case 0x31: 'N';
                        case 0x32: 'M';
                        case 0x33: '<';
                        case 0x34: '>';
                        case 0x35: '?';
                        case 0x37: '*';
                        case 0x39: ' ';
                        
                    }
                case 0b1110: // MOD + ALT + CTRL
                    switch (keycode) {
                        case 0x02: '1';
                        case 0x03: '2';
                        case 0x04: '3';
                        case 0x05: '€';
                        case 0x06: '5';
                        case 0x07: '6';
                        case 0x08: '7';
                        case 0x09: '8';
                        case 0x0A: '9';
                        case 0x0B: '0';
                        case 0x0C: '-';
                        case 0x0D: '=';
                        case 0x10: 'q';
                        case 0x11: 'w';
                        case 0x12: 'é';
                        case 0x13: 'r';
                        case 0x14: 't';
                        case 0x15: 'y';
                        case 0x16: 'ú';
                        case 0x17: 'í';
                        case 0x18: 'ó';
                        case 0x19: 'p';
                        case 0x1A: '[';
                        case 0x1B: ']';
                        case 0x1E: 'á';
                        case 0x1F: 's';
                        case 0x20: 'd';
                        case 0x21: 'f';
                        case 0x22: 'g';
                        case 0x23: 'h';
                        case 0x24: 'j';
                        case 0x25: 'k';
                        case 0x26: 'l';
                        case 0x27: ';';
                        case 0x28: '\'';
                        case 0x2A: '¦';
                        case 0x2B: '#';
                        case 0x2C: 'z';
                        case 0x2D: 'x';
                        case 0x2E: 'c';
                        case 0x2F: 'v';
                        case 0x30: 'b';
                        case 0x31: 'n';
                        case 0x32: 'm';
                        case 0x33: ',';
                        case 0x34: '.';
                        case 0x35: '/';
                        case 0x37: '*';
                        case 0x39: ' ';
                        
                    }
                case 0b1111: // MOD + ALT + CTRL + SHIFT
                    switch (keycode) {
                        case 0x02: '!';
                        case 0x03: '"';
                        case 0x04: '£';
                        case 0x05: '$';
                        case 0x06: '%';
                        case 0x07: '^';
                        case 0x08: '&';
                        case 0x09: '*';
                        case 0x0A: '(';
                        case 0x0B: ')';
                        case 0x0C: '_';
                        case 0x0D: '+';
                        case 0x10: 'Q';
                        case 0x11: 'W';
                        case 0x12: 'É';
                        case 0x13: 'R';
                        case 0x14: 'T';
                        case 0x15: 'Y';
                        case 0x16: 'Ú';
                        case 0x17: 'Í';
                        case 0x18: 'Ó';
                        case 0x19: 'P';
                        case 0x1A: '{';
                        case 0x1B: '}';
                        case 0x1E: 'Á';
                        case 0x1F: 'S';
                        case 0x20: 'D';
                        case 0x21: 'F';
                        case 0x22: 'G';
                        case 0x23: 'H';
                        case 0x24: 'J';
                        case 0x25: 'K';
                        case 0x26: 'L';
                        case 0x27: ':';
                        case 0x28: '@';
                        case 0x2A: '¬';
                        case 0x2B: '~';
                        case 0x2C: 'Z';
                        case 0x2D: 'X';
                        case 0x2E: 'C';
                        case 0x2F: 'V';
                        case 0x30: 'B';
                        case 0x31: 'N';
                        case 0x32: 'M';
                        case 0x33: '<';
                        case 0x34: '>';
                        case 0x35: '?';
                        case 0x37: '*';
                        case 0x39: return ' ';
                        
                    }
            }
    }

    return 'A';
}