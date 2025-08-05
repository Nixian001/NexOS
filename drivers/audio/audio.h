#pragma once

float midi_to_freq(uint8_t midi);
void play_sound(float nFrequency);
void stop_sound();

// Basic SFX
void beep();
void nexos_boot_audio();