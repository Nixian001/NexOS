#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#define low_16(address) (uint16_t)((address) & 0xFFFF)
#define high_16(address) (uint16_t)(((address) >> 16) & 0xFFFF)

void util_init();

void util_quit();

int compare_string(char s1[], char s2[]);

int string_length(char s[]);

void reverse(char s[]);

bool backspace(char buffer[]);

void int_to_string(int n, char str[]);

char* dec_to_bin(int n, int len);