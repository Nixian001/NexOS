#include <stdint.h>

/**
 * Read a byte from the specified port
 */
uint8_t  port_byte_in_8  (uint16_t port) {
    unsigned char result;
    /* Inline assembler syntax
     * !! Notice how the source and destination registers are switched from NASM !!
     *
     * '"=a" (result)'; set '=' the C variable '(result)' to the value of register e'a'x
     * '"d" (port)': map the C variable '(port)' into e'd'x register
     *
     * Inputs and outputs are separated by colons
     */
    asm("in %%dx, %%al" : "=a" (result) : "d" (port));
    return result;
}

uint16_t port_bytes_in_16(uint16_t port) {
    unsigned short result;
    asm("in %%dx, %%ax" : "=a" (result) : "d" (port));
    return result;
}

uint32_t port_bytes_in_32(uint16_t port) {
    uint32_t result;
    asm("in %%dx, %%eax" : "=a" (result) : "d" (port));
    return result;
}

void port_byte_out_8 (uint16_t port, uint8_t data) {
    /* Notice how here both registers are mapped to C variables and
     * nothing is returned, thus, no equals '=' in the asm syntax
     * However we see a comma since there are two variables in the input area
     * and none in the 'return' area
     */
    asm("out %%al, %%dx" : : "a" (data), "d" (port));
}

void port_word_out_16(uint16_t port, uint16_t data) {
    asm("out %%ax, %%dx" : : "a" (data), "d" (port));
}

void port_word_out_32(uint16_t port, uint32_t data) {
    asm("out %%eax, %%dx" : : "a" (data), "d" (port));
}