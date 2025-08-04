#include "keyboard.h"
#include "ports.h"
#include "../cpu/isr.h"
#include "display.h"
#include "../apps/shell.h"
#include "../kernel/util.h"

static char key_buffer[256];

// const char scancode_to_char[] = {
//     '?', '`', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',
//     '?', '?', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']',
//     '?', '?', 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '#',
//    '?', '\\', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '?',
//                 '?', '?', ' '
// };

const char scancode_to_char[] = {
  '?', '`', '1', '2', '3', '4', '5',
  '6', '7', '8', '9', '0', '-', '=',
  '?', '?', 'Q', 'W', 'E', 'R', 'T',
  'Y', 'U', 'I', 'O', 'P', '[', ']',
  '?', '?', 'A', 'S', 'D', 'F', 'G',
  'H', 'J', 'K', 'L', ';', '\'', '#',
  '?', '\\', 'Z', 'X', 'C', 'V', 'BW',
  'N', 'M', ',', '.', '/', '?', '?',
  '?', ' '
};


void print_letter(uint8_t scancode) {
    switch (scancode) {
        case 0x0:
            print_string("ERROR");
            break;
        case 0x1:
            print_string("ESC");
            break;
        case 0x2:
            print_string("1");
            break;
        case 0x3:
            print_string("2");
            break;
        case 0x4:
            print_string("3");
            break;
        case 0x5:
            print_string("4");
            break;
        case 0x6:
            print_string("5");
            break;
        case 0x7:
            print_string("6");
            break;
        case 0x8:
            print_string("7");
            break;
        case 0x9:
            print_string("8");
            break;
        case 0x0A:
            print_string("9");
            break;
        case 0x0B:
            print_string("0");
            break;
        case 0x0C:
            print_string("-");
            break;
        case 0x0D:
            print_string("+");
            break;
        case 0x0E:
            print_string("Backspace");
            break;
        case 0x0F:
            print_string("Tab");
            break;
        case 0x10:
            print_string("Q");
            break;
        case 0x11:
            print_string("W");
            break;
        case 0x12:
            print_string("E");
            break;
        case 0x13:
            print_string("R");
            break;
        case 0x14:
            print_string("T");
            break;
        case 0x15:
            print_string("Y");
            break;
        case 0x16:
            print_string("U");
            break;
        case 0x17:
            print_string("I");
            break;
        case 0x18:
            print_string("O");
            break;
        case 0x19:
            print_string("P");
            break;
        case 0x1A:
            print_string("[");
            break;
        case 0x1B:
            print_string("]");
            break;
        case 0x1C:
            print_string("ENTER");
            break;
        case 0x1D:
            print_string("LCtrl");
            break;
        case 0x1E:
            print_string("A");
            break;
        case 0x1F:
            print_string("S");
            break;
        case 0x20:
            print_string("D");
            break;
        case 0x21:
            print_string("F");
            break;
        case 0x22:
            print_string("G");
            break;
        case 0x23:
            print_string("H");
            break;
        case 0x24:
            print_string("J");
            break;
        case 0x25:
            print_string("K");
            break;
        case 0x26:
            print_string("L");
            break;
        case 0x27:
            print_string(";");
            break;
        case 0x28:
            print_string("'");
            break;
        case 0x29:
            print_string("`");
            break;
        case 0x2A:
            print_string("LShift");
            break;
        case 0x2B:
            print_string("\\");
            break;
        case 0x2C:
            print_string("Z");
            break;
        case 0x2D:
            print_string("X");
            break;
        case 0x2E:
            print_string("C");
            break;
        case 0x2F:
            print_string("V");
            break;
        case 0x30:
            print_string("B");
            break;
        case 0x31:
            print_string("N");
            break;
        case 0x32:
            print_string("M");
            break;
        case 0x33:
            print_string(",");
            break;
        case 0x34:
            print_string(".");
            break;
        case 0x35:
            print_string("/");
            break;
        case 0x36:
            print_string("Rshift");
            break;
        case 0x37:
            print_string("Keypad *");
            break;
        case 0x38:
            print_string("LAlt");
            break;
        case 0x39:
            print_string("Space");
            break;
        default:
            /* 'keuyp' event corresponds to the 'keydown' + 0x80
             * it may still be a scancode we haven't implemented yet, or
             * maybe a control/escape sequence */
            if (scancode <= 0x7f) {
                print_string("Unknown key down");
            } else if (scancode <= 0x39 + 0x80) {
                print_string("key up ");
                print_letter(scancode - 0x80);
            } else print_string("Unknown key up");
            break;
    }
}

static void keyboard_callback(registers_t *regs) {
    uint8_t scancode = port_byte_in(0x60);

    if (scancode > SC_MAX) return;

    if (scancode == BACKSPACE) {
        if (backspace(key_buffer)) {
            print_backspace();
        }
    } else if (scancode == ENTER) {
        print_nl();
        execute_command(key_buffer);
        key_buffer[0] = '\0';
    } else {
        char letter = scancode_to_char[(int) scancode];
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
