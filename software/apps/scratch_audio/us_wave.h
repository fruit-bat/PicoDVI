#pragma once
#include "us_tuner.h"

typedef int32_t (*UsWaveFunc)(
    const uint32_t bang
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

int32_t us_wave_ramp_up(
    const uint32_t bang
);

int32_t us_wave_ramp_down(
    const uint32_t bang
);
