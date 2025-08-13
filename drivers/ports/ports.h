#pragma once

#include <stdint.h>

uint8_t  port_byte_in_8  (uint16_t port);
uint16_t port_bytes_in_16(uint16_t port);
uint32_t port_bytes_in_32(uint16_t port);

void port_byte_out_8 (uint16_t port, uint8_t data);
void port_word_out_16(uint16_t port, uint16_t data);
void port_word_out_32(uint16_t port, uint32_t data);