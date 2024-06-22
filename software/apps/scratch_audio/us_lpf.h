#pragma once
#include "pico/stdlib.h"

typedef struct {
    int32_t prev;
    uint32_t alpha;
} UsLpf;

void us_lpf_init(UsLpf *filter, uint32_t alpha);

int32_t us_lpf_sample(UsLpf *filter, int32_t s);
