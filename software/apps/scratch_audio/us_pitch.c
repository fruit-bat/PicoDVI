#include "us_pitch.h"
#include "us_config.h"
#include "us_pitch_table.h"

// This should work up to 4104
// We could use this to reduce the size of the tone tables... but not sure it is worth it.
static inline int32_t us_divide_by_12_approx(uint32_t n) {
    return (__mul_instruction(n, 1365) + 1364) >> 14;
}

// Select a tone from the table, no lerp
void us_set_midi_note(
    UsPitch *pitch,             // Pitch to set
    const uint32_t note_index,  // MIDI note number, range 0, 127
    const int32_t pitch_bend)   // 0 = no bend, range +/- 16384, for 2 tones
{
    const int32_t fni = (int32_t)((US_NOTE_MIDI_OFFSET + note_index) << US_NOTE_FRACTIONS_LOG2) + (pitch_bend >> 7);
    const int32_t mni = (fni >> US_NOTE_FRACTIONS_LOG2);
    const uint32_t octave = us_divide_by_12_approx(mni);
    const int32_t ani = fni - (__mul_instruction(octave, US_NOTE_TABLE_SIZE));
    pitch->eips = us_pitch_table_eips[ani] + octave;
    pitch->fips = us_pitch_table_fips[ani];
}

// Not sure how accurate this is. Use with caution.
void us_set_freqency_hz(
    UsPitch *pitch,
    uint32_t fhz // fixed point 16,16
) {
    // Assume f/fs < 1
    uint64_t t1 = (((uint64_t)fhz) << 32ULL) / US_SAMPLE_RATE_HZ;
    // Shift right until only 31 bits are used (slow)
    int32_t e = 0;
    while(t1 > ((1UL << 31UL) - 1UL)) {
        e++;
        t1 >>= 1;
    }
    pitch->fips = t1;
    pitch->eips = 32 - e;
}

// Not sure how accurate this is. Use with caution.
void us_set_wavelength_ms(
    UsPitch *pitch,
    uint16_t ms 
) {
    // // Assume f/fs < 1
    // uint64_t t1 = ((1000ULL << 32ULL) / __mul_instruction(ms, US_SAMPLE_RATE_HZ);
    // // Shift right until only 31 bits are used (slow)
    // int32_t e = 0;
    // while(t1 > ((1UL << 31UL) - 1UL)) {
    //     e++;
    //     t1 >>= 1;
    // }
    // pitch->fips = t1;
    // pitch->eips = 32 - e;
}
