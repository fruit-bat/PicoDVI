#pragma once
#include "pico/stdlib.h"

// f  = frequency
// sf = sample frequency
// be = binary exponent
typedef struct {
    uint32_t fips; // Increment per sample ((2^(32+be))*f/sf)
    uint8_t eips;  // Negative binary exponent of fips (-be)
    uint32_t facc; // Fractional accumulator
    uint32_t bang; // Binary anngle. Complete rotation = 2^32 
} UsTuner;

inline void us_tuner_reset(
    UsTuner *tuner
) {
    tuner->facc = 0;
    tuner->bang = 0;
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