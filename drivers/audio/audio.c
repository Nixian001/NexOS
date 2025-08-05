#include <stdint.h>
#include <math.h>
#include "audio.h"
#include "../../cpu/timer.h"
#include "../ports/ports.h"
#include "../../kernel/util.h"

float expf(float x) {
    float result = 1.0f;
    float term = 1.0f;
    int n = 1;

    while (term > 1e-6f || term < -1e-6f) {
        term *= x / n;
        result += term;
        n++;
    }
    return result;
}

float lnf(float x) {
    if (x <= 0.0f) {
        return -INFINITY; // Logarithm undefined for non-positive values
    }
    float result = 0.0f;
    float term = (x - 1.0f) / (x + 1.0f);
    float term_squared = term * term;
    float current_term = term;

    for (int n = 1; n < 100000; n += 2) {
        result += current_term / n;
        current_term *= term_squared;
    }
    return 2.0f * result;
}

float powf(float base, float exp) {
    return expf(exp * lnf(base));
}

float midi_to_freq(uint8_t midi) {
    return 440.0f * powf(2.0f, (midi - 49) / 12.0f);
}

void play_sound(float nFrequency) {
    uint32_t div;
    uint8_t tmp;

    if (nFrequency < 20.0f)
        nFrequency = 20.0f; // Minimum frequency
    if (nFrequency > 20000.0f)
        nFrequency = 20000.0f; // Maximum frequency
    
    div = 1193180 / nFrequency;
    port_byte_out(0x43, 0xB6);
    port_byte_out(0x42, (uint8_t) div);
    port_byte_out(0x42, (uint8_t) (div >> 8));

    tmp = port_byte_in(0x61);
    if (tmp != (tmp | 3)) {
        port_byte_out(0x61, tmp | 3);
    }
}

void stop_sound() {
    uint8_t tmp = port_byte_in(0x61) & 0xFC;
    port_byte_out(0x61, tmp);
}

void beep() {
    play_sound(100);   // Play sound at 1000 Hz
    timer_sleep(10);    // Sleep for 1 ms
    stop_sound();       // Stop sound
}

void nexos_boot_audio() {
    // C4 G4 E5 C5 F4 E4 C4
    play_sound(midi_to_freq(60));  // C4
    timer_sleep(100);
    play_sound(midi_to_freq(67));  // G4
    timer_sleep(100);
    play_sound(midi_to_freq(76));  // E5
    timer_sleep(100);
    play_sound(midi_to_freq(72));  // C5
    timer_sleep(100);
    play_sound(midi_to_freq(65));  // F4
    timer_sleep(100);
    play_sound(midi_to_freq(64));  // E4
    timer_sleep(100);
    play_sound(midi_to_freq(60));  // C4
    timer_sleep(200);
    stop_sound();  // Stop sound after the melody
}