#pragma once

#define VIDEO_ADDRESS 0xB8000
#define MAX_ROWS 25
#define MAX_COLS 80
#define WHITE_ON_BLACK 0x02

/* Screen i/o ports */
#define REG_SCREEN_CTRL 0x3D4
#define REG_SCREEN_DATA 0x3D5

void printf(const char* format, ...);

/* Public kernel API */
void print_string(char* string);
void print_nl();
void print_backspace();
void print_int(int n);
void clear_screen();
int scroll_ln(int offset);

void set_cursor(int offset);
int get_offset(int col, int row);