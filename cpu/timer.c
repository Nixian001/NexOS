#include "timer.h"
#include "../drivers/display/display.h"
#include "../drivers/ports/ports.h"
#include "../kernel/util.h"
#include "isr.h"

uint32_t tick = 0;
static uint32_t TIMER_FREQ; // Default timer frequency in Hz

static void timer_callback(registers_t *regs) {
    tick++;
}

void init_timer(uint32_t freq) {
    TIMER_FREQ = freq;

    /* Install the function we just wrote */
    register_interrupt_handler(IRQ0, timer_callback);

    /* Get the PIT value: hardware clock at 1193180 Hz */
    uint32_t divisor = 1193180 / freq;
    uint8_t low  = (uint8_t)(divisor & 0xFF);
    uint8_t high = (uint8_t)( (divisor >> 8) & 0xFF);
    /* Send the command */
    port_byte_out(0x43, 0x36); /* Command port */
    port_byte_out(0x40, low);
    port_byte_out(0x40, high);
}

void timer_sleep(uint16_t ms) {
    // Convert ms to ticks based on the PIT frequency
    uint32_t start_tick = tick;
    uint32_t wait_ticks = (uint32_t)((ms * TIMER_FREQ) / 1000); // if TIMER_FREQ is in Hz

    // print_string("Sleeping for ");
    // char wait_ticks_str[8];
    // int_to_string(wait_ticks, wait_ticks_str);
    // print_string(wait_ticks_str);
    // print_string(" ticks.\n");

    do {
        // Busy-wait until the desired tick count is reached
    } while (tick < start_tick + wait_ticks);
}
