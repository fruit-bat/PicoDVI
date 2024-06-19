#pragma once
#include "pico/stdlib.h"
#include "us_tuner.h"

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

typedef int32_t (*UsBangToWave)(
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



