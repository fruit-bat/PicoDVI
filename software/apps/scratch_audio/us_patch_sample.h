#pragma once
#include "us_patch.h"

typedef struct {
    uint32_t pos;
} UsPatchSampleData;

typedef struct {
    uint16_t *samples;
    uint32_t sample_count;
} UsPatchSampleConfig;

void us_patch_sample_apply(UsPatch *patch);
