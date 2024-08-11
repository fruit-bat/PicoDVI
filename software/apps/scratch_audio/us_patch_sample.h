#pragma once
#include "us_patch.h"

typedef struct {
    uint32_t pos;
} UsPatchSampleData;

typedef struct {
    int16_t *samples;
    uint32_t sample_count;
} UsPatchSampleConfig;

void us_patch_sample_init_config(UsPatchSampleConfig* patch_config, const int16_t *samples, uint32_t sample_count);
void us_patch_sample_apply(UsPatch *patch);
