#pragma once
#include "us_patch.h"
#include "us_tuner.h"
#include "us_wave.h"
#include "us_adsr.h"

typedef struct {
    UsTuner tuner;
    UsAdsr adsr;
} UsPatch1Data;

typedef struct {
    UsAdsrConfig adsr_config;
    UsWaveFunc wave_func;
} UsPatch1Config;

void us_patch_1_init_config(UsPatch1Config* config);
void us_patch_1_apply(UsPatch *patch);
