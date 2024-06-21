#pragma once
#include "pico/stdlib.h"
#include "us_wave.h"


typedef int32_t (*UsBangToWave)(
    const uint32_t bang
);

typedef struct {
    UsBangToWave bangToWave;
    UsTuner *tuner;
} UsWave;




