#include "u_synth.h"

typedef int32_t fix;
#define multfix(a,b) ((fix))(((( signed long long a))*((signed long long b)) >> 15))
#define dec2fix(a) ((fix)(a * 32768.0d))
#define frac2us(a) ((uint32_t)(1000000d*a))

#define US_SAMPLE_RATE_HZ 44100
#define US_BANG_MAX ((uint32_t)-1l)

#include "u_synth_tone_tables.h"

void __not_in_flash_func(us_set_pitch_from_tables)(
    UsTuner *tuner,
    uint32_t ni
){
    tuner->eips = us_bas[ni];
    tuner->fips = us_bae[ni];
}

uint32_t inline us_tick_facc(
   UsTuner *tuner
) {
    const uint32_t eips = (uint32_t)tuner->eips;
    const uint32_t facc = tuner->facc + tuner->fips;
    const uint32_t hacc = facc >> eips;
    tuner->facc = facc - (hacc << eips);
    return hacc;
}

inline void us_tuner_reset(
    UsTuner *tuner
) {
    tuner->facc = 0;
    tuner->bang = 0;
}

// Update for single sample
void __not_in_flash_func(us_tick)(
    UsTuner *tuner
) {
    tuner->bang += us_tick_facc(tuner);
}

// Update for single sample return true if wrapped
bool __not_in_flash_func(us_tick_check_wrap)(
    UsTuner *tuner
) {
    const uint32_t bang_prev = tuner->bang;
    const uint32_t bang_next = bang_prev + us_tick_facc(tuner);
    const bool wrap = bang_next < bang_prev;
    tuner->bang = bang_next;
    return wrap;
}

void us_set_freqency_hz(
    UsTuner *tuner,
    uint32_t fhz
) {
    // Assume f/fs < 1
 //   uint64_t t1 = (((uint64_t)fhz) << 32) / ((uint64_t)US_SAMPLE_RATE_HZ);
}

inline static int32_t us_lerp_n(
    const int32_t s1,           // Sample 1
    const int32_t s2,           // Sample 2
    const uint32_t n_bit_frac,  // n bits of fraction
    const uint32_t n            // number of fraction bits
) {
    const int32_t d = __mul_instruction(s2 - s1, n_bit_frac) >> n;
    return s1 + d;
}

inline static int16_t us_wave_sin_sample_q(
    const uint32_t qang,
    const uint32_t qind
) {
    switch(qang) {
        case 0: return us_sin[qind];
        case 1: return us_sin[128 - qind];
        case 2: return - us_sin[qind];
        default: return - us_sin[128 - qind];
    }
}

inline static int16_t us_wave_sin_sample(
    const uint32_t sang
) {
    const uint32_t qang = sang >> 7;
    const uint32_t qind = sang & 127;
    return us_wave_sin_sample_q(qang, qind);
}

int16_t __not_in_flash_func(us_wave_sin_lerp)(
    const uint32_t bang
) {
    const uint32_t zang = bang >> (32 - 2 - 7 - 8);
    const uint32_t fang = zang & 255;
    const uint32_t sang1 = zang >> 8;
    const int32_t s1 = us_wave_sin_sample(sang1);
    const uint32_t sang2 = (sang1 + 1) & ((1 << (2 + 7)) - 1);
    const int32_t s2 = us_wave_sin_sample(sang2);
    return (int16_t)us_lerp_n(s1, s2, fang, 8);
}

int16_t __not_in_flash_func(us_wave_sin)(
    const uint32_t bang
) {
    //const uint32_t sang = bang >> (32 - 2 - 7);
    //return (int16_t)us_wave_sin_sample(sang);
    const uint32_t qind = bang >> (32 - 2);
    const uint32_t qang = (bang >> (32 - 2 - 7)) & 127;
    switch(qang) {
        case 0: return us_sin[qind];
        case 1: return us_sin[128 - qind];
        case 2: return - us_sin[qind];
        default: return - us_sin[128 - qind];
    } 
}

int16_t __not_in_flash_func(us_wave_saw)(
    const uint32_t bang
) {
    const uint32_t qang = bang >> (32 - 2);
    const uint32_t qind = (bang >> (32 - 2 - 15)) && 32767;
    switch(qang) {
        case 0: return qind;
        case 1: return 32767 - qind;
        case 2: return - qind;
        default: return qind - 32768;
    }
}

int16_t __not_in_flash_func(us_tick_envelope)(
    UsEnvelope *envelope
) {
    switch(envelope->stage) {
        // Attack
        case 0: {
            uint32_t bang;
            if (us_tick_check_wrap(envelope->attack.tuner)) {
                bang = US_BANG_MAX;
                envelope->stage = 1;
            }
            else {
                bang = envelope->attack.tuner->bang;
            }
            const int16_t v = envelope->decay.bangToWave(bang);
            envelope->sustain = v;
            return v;
        }
        // Decay
        case 1: {
            if (us_tick_check_wrap(envelope->decay.tuner)) {
                us_tuner_reset(envelope->decay.tuner);
                const int16_t v = envelope->decay.bangToWave(US_BANG_MAX);
                envelope->stage = 2;
                envelope->sustain = v;
                return v;
            }
            else {
                return envelope->decay.bangToWave(envelope->decay.tuner->bang);
            }
        }
        // Sustain
        case 2: {
            return envelope->sustain;
        }
        // Release
        case 3: {
            if (us_tick_check_wrap(envelope->decay.tuner)) {
                us_tuner_reset(envelope->decay.tuner);
                const int16_t v = envelope->decay.bangToWave(US_BANG_MAX);
                envelope->stage = 2;
                envelope->sustain = v;
                return v;
            }
            else {
                return envelope->decay.bangToWave(envelope->decay.tuner->bang);
            }
        }
    }
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

