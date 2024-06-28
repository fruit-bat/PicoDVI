#pragma once
#include "pico/stdlib.h"

typedef struct {
    uint32_t fips; // Increment per sample ((2^(32+be))*f/sf)
    uint32_t eips; // Negative binary exponent of fips (-be)
} UsPitch;
