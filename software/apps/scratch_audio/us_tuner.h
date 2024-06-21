#pragma once
#include "pico/stdlib.h"
#include "us_pitch.h"

#define US_BANG_MAX ((uint32_t)-1l)

// f  = frequency
// sf = sample frequency
// be = binary exponent
typedef struct {
    uint32_t fips; // Increment per sample ((2^(32+be))*f/sf)
    uint32_t eips; // Negative binary exponent of fips (-be)
    uint32_t facc; // Fractional accumulator
    uint32_t bang; // Binary anngle. Complete rotation = 2^32 
} UsTuner;

inline void us_tuner_reset_phase(
    UsTuner *tuner
) {
    tuner->facc = 0;
    tuner->bang = 0;
}

inline void us_tuner_set_pitch(
    UsTuner *tuner,
    UsPitch *pitch
) {
    tuner->fips = pitch->fips;
    tuner->eips = pitch->eips;
}

void us_tuner_rotate(
    UsTuner *tuner
);

bool us_tuner_rotate_check_wrap(
    UsTuner *tuner
);

void us_tuner_set_note(
    UsTuner *tuner,
    uint32_t note
);