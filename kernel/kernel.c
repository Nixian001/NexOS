#include "../cpu/idt.h"
#include "../cpu/isr.h"
#include "../cpu/timer.h"
#include "../drivers/display.h"
#include "../drivers/keyboard.h"

#include "util.h"

void main() {
    clear_screen();
    print_string("Installing interrupt service routines (ISRs).\n");
    isr_install();

    print_string("Enabling external interrupts.\n");
    asm volatile("sti");

    print_string("Initializing keyboard (IRQ 1).\n");
    init_keyboard();

    clear_screen();

    print_nl();
    print_string("  888b    |                      ,88~-_   ,d88~~\\");
    print_nl();
    print_string("  |Y88b   |  e88~~8e  Y88b  /   d888   \\  8888   ");
    print_nl();
    print_string("  | Y88b  | d888  88b  Y88b/   88888    | `Y88b  ");
    print_nl();
    print_string("  |  Y88b | 8888__888   Y88b   88888    |  `Y88b,");
    print_nl();
    print_string("  |   Y88b| Y888    ,   /Y88b   Y888   /     8888");
    print_nl();
    print_string("  |    Y888  \"88___/   /  Y88b   `88_-~   \\__88P'");
    print_nl();
    print_nl();
    print_nl();

    print_string("> ");
}