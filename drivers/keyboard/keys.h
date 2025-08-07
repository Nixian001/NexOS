#pragma once

#define SHIFT_HELD      0b00000001
#define CTRL_HELD       0b00000010
#define ALT_HELD        0b00000100
#define MOD_HELD        0b00001000
#define CAPSLOCK_TOG    0b00010000
#define NUM_LOCK_TOG    0b00100000
#define MOD_LOCK_TOG    0b01000000

#define L_SHIFT 0x2A
#define R_SHIFT 0x36
#define CTRL 0x1D
#define ALT 0x38
#define MOD 0x5D

#define CAPSLOCK 0x3A
#define NUMLOCK 0x45
#define MODLOCK 0x46

#define KEYMODIFS       0b01011111 // mask which holds what modifs change actual keys

enum KEYS {
    K_NULL,
    K_ESCAPE,

    K_1,
    K_2,
    K_3,
    K_4,
    K_5,
    K_6,
    K_7,
    K_8,
    K_9,
    K_0,

    K_MINUS,
    K_EQUALS,
    K_BACKSPACE,
    
    K_TAB,
    K_Q,
    K_W,
    K_E,
    K_R,
    K_T,
    K_Y,
    K_U,
    K_I,
    K_O,
    K_P,
    K_LEFTBRACKET,
    K_RIGHTBRACKET,
    K_ENTER,

    K_CONTROL,

    K_A,
    K_S,
    K_D,
    K_F,
    K_G,
    K_H,
    K_J,
    K_K,
    K_L,
    K_COLON,
    K_QUOTE,
    K_LEFTSHIFT,
    K_BACKQUOTE,
    K_HASH,

    K_Z,
    K_X,
    K_C,
    K_V,
    K_B,
    K_N,
    K_M,
    K_COMMA,
    K_FULLSTOP,
    K_SLASH,
    K_RIGHTSHIFT,

    NUM_ASTERISK,

    K_ALT,
    K_SPACE,
    K_CAPSLOCK,

    F_1,
    F_2,
    F_3,
    F_4,
    F_5,
    F_6,
    F_7,
    F_8,
    F_9,
    F_10,
    
    NUM_NUMLOCK,
    F_SCROLLLOCK,

    NUM_7,
    NUM_8,
    NUM_9,
    NUM_MINUS,
    NUM_4,
    NUM_5,
    NUM_6,
    NUM_PLUS,
    NUM_1,
    NUM_2,
    NUM_3,
    NUM_0,
    NUM_PERIOD,

    F_11 = 0x57,
    F_12 = 0x58,

    K_APP = 0x5D
};



void keys_debug(int keycode);

char keycode_to_char(int keycode);