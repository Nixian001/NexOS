#include "../cpu/idt.h"
#include "../cpu/isr.h"
#include "../cpu/timer.h"
#include "../drivers/display/display.h"
#include "../drivers/keyboard/keyboard.h"
#include "../drivers/audio/audio.h"

#include "util.h"
#include "mem.h"

void main() {

    clear_screen();

    init_timer(1000); // Initialize the timer with a frequency of 1000 Hz

    print_string("Installing interrupt service routines (ISRs).\n");
    isr_install();

    print_string("Enabling external interrupts.\n");
    asm volatile("sti");

    print_string("Initializing keyboard (IRQ 1).\n");
    init_keyboard();

    clear_screen();

    print_nl();
    print_string("  88b    |                    ,88~-_   ,d8~~\\"); print_nl();
    print_string("  |Y8b   |  e8~~8e  Y8b  /   d88    \\  888   "); print_nl();
    print_string("  | Y8b  | d88  88b  Y8b/   8888     | `Y8b  "); print_nl();
    print_string("  |  Y8b | 888__888   Y8b   8888     |  `Y8b,"); print_nl();
    print_string("  |   Y8b| Y88    ,   /Y8b   Y88    /     888"); print_nl();
    print_string("  |    Y88  \"8___/   /  Y8b   `88_-~   \\__8P'");
    print_nl();
    print_nl();
    print_nl();

    nexos_boot_audio();

    print_string("> ");
}