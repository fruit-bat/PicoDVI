#include "u_synth.h"

typedef int32_t fix;
#define multfix(a,b) ((fix))(((( signed long long a))*((signed long long b)) >> 15))
#define dec2fix(a) ((fix)(a * 32768.0d))
#define frac2us(a) ((uint32_t)(1000000d*a))

#define US_SAMPLE_RATE_HZ 44100

#include "u_synth_tone_tables.h"



void __not_in_flash_func(us_set_pitch_from_tables)(
    USTuner *tuner,
    uint32_t ni
){
    tuner->eips = us_bas[ni];
    tuner->fips = us_bae[ni];
}

// Update for single sample
void __not_in_flash_func(us_tick)(
    USTuner *tuner
) {
    const uint32_t eips = (uint32_t)tuner->eips;
    const uint32_t facc = tuner->facc + tuner->fips;
    const uint32_t hacc = facc >> eips;
    tuner->bang += hacc;
    tuner->facc = facc - (hacc << eips);
}

void us_set_freqency_hz(
    USTuner *tuner,
    uint32_t fhz
) {
    // Assume f/fs < 1
 //   uint64_t t1 = (((uint64_t)fhz) << 32) / ((uint64_t)US_SAMPLE_RATE_HZ);
}


// This is like a sample but is a combination of a amplitude and a time
typedef struct {
    fix a;
    fix t;
} UsWaveDelta;

typedef struct {
    fix r;        // a fixed point rate
    uint32_t lg2; // a us clock divider
} UsWaveRate;

typedef struct {
    UsWaveDelta *wd; // A waveform
    uint32_t wds;    // Number of entries in the waveform
    UsWaveRate r;    // How fast to play the waveform
    fix x;           // Position in the waveform
} UsWaveCursor;

// Cyclic Waveforms start and end on 0 and range from 1 to -1
// A waveform cycle is defined at 1Hz
// Adjust the clock to play the waveform
UsWaveDelta wave_form_square[] = {
    { dec2fix(1), dec2fix(0.0) },
    { dec2fix(1), dec2fix(0.5) },
    { dec2fix(-1), dec2fix(0.5) },
    { dec2fix(-1), dec2fix(1.0) },
};

UsWaveDelta wave_form_triangle[] = {
    { dec2fix(1), dec2fix(0.25) },
    { dec2fix(-1), dec2fix(0.75) },
    { dec2fix(0), dec2fix(1.0) },
};

void us_set_rate(fix hz, UsWaveRate *r) {
    // Assume we have a 44100 sample rate
    // A sample is taken every ~22.67us

    // to play a wave a 1Hz
}

void wave_step() {

}
