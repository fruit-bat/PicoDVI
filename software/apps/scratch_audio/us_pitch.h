#pragma once
#include "pico/stdlib.h"

// f  = frequency
// sf = sample frequency
// be = binary exponent
typedef struct {
    uint32_t fips; // Increment per sample ((2^(32+be))*f/sf)
    uint32_t eips; // Negative binary exponent of fips (-be)
} UsPitch;

// Select a tone from the table, no lerp
void us_pitch_set_midi_note(
    UsPitch *pitch,             // Pitch to set
    const uint32_t note_index,  // MIDI note number, range 0, 127
    const int32_t pitch_bend);  // 0 = no bend, range +/- 16384, for 2 tones

inline void us_pitch_copy(
    UsPitch *to,
    UsPitch *from
) {
    to->fips = from->fips;
    to->eips = from->eips;
}
