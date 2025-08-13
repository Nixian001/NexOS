#pragma once

#include <stdint.h>
#include <stdbool.h>

#define low_16(address) (uint16_t)((address) & 0xFFFF)
#define high_16(address) (uint16_t)(((address) >> 16) & 0xFFFF)

int compare_string(char s1[], char s2[]);
void itoa(char *buf, unsigned long int n, int base);
int atoi(char * string);
char *string_copy(char *dst,const char *src);

int string_length(char s[]);

void reverse(char s[]);

bool backspace(char buffer[]);

void int_to_string(int n, char str[]);