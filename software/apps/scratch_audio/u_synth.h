#pragma once
#include "pico/stdlib.h"

#define US_CHANNELS 4

typedef uint8_t us_channel;

// f  = frequency
// sf = sample frequency
// be = binary exponent
typedef struct {
    uint32_t fips; // Increment per sample ((2^(32+be))*f/sf)
    uint8_t eips;  // Negative binary exponent of fips (-be)
    uint32_t facc; // Fractional accumulator
    uint32_t bang; // Binary anngle. Complete rotation = 2^32 
} USTuner;

void us_tick(
    USTuner *tuner
);

void us_set_pitch_from_tables(
    USTuner *tuner,
    uint32_t ni
);

int16_t us_wave_sin(
    USTuner *tuner
);