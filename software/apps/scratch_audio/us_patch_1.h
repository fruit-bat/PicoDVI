#pragma once
#include "us_patch.h"
#include "us_tuner.h"
#include "us_wave.h"
#include "us_adsr.h"

typedef struct {
    UsTuner tuner;
    UsAdsr adsr;
} UsPatch1;

typedef struct {
    UsAdsrConfig adsr_config;
    UsWaveFunc wave_func;
} UsPatch1Config;

// TODO how does the config get initialised?

extern UsPatch patch1;

