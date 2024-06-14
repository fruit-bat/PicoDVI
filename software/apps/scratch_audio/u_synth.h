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
} UsTuner;

void us_tick(
    UsTuner *tuner
);

bool us_tick_check_wrap(
    UsTuner *tuner
);

void us_set_pitch_from_tables(
    UsTuner *tuner,
    uint32_t ni
);

int32_t us_wave_sin(
    const uint32_t bang
);

int32_t us_wave_sin_lerp(
    const uint32_t bang
);

int32_t us_wave_saw(
    const uint32_t bang
);

int32_t us_wave_square(
    const uint32_t bang
);

typedef int16_t (*UsBangToWave)(
    const uint32_t bang
);

typedef struct {
    UsBangToWave bangToWave;
    UsTuner *tuner;
} UsWave;

typedef struct {
    UsWave attack;    // Attack
    UsWave decay;     // Decay
    UsWave release;   // Release
    uint16_t sustain; // gain
    uint8_t stage;    // stage
} UsEnvelope;



