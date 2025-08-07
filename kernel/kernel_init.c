#include "kernel_init.h"
#include "mem.h"
#include "util.h"
#include "../drivers/display/display.h"
#include "../cpu/timer.h"
#include "../cpu/isr.h"
#include "../drivers/keyboard/keyboard.h"

void kernel_init() {
    clear_screen();

    init_timer(1000); // Initialize the timer with a frequency of 1000 Hz

    print_string("Installing interrupt service routines (ISRs).\n");
    isr_install();

    print_string("Enabling external interrupts.\n");
    asm volatile("sti");

    print_string("Creating Memory.\n");
    init_dynamic_mem();

    print_string("Allocating Memory to Vitals");
    util_init();

    print_string("Initializing keyboard (IRQ 1).\n");
    init_keyboard();
}