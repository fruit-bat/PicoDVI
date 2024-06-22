#include "us_pitch.h"
#include "us_config.h"

// Not sure how accurate this is. Use with caution.
void us_set_freqency_hz(
    UsPitch *pitch,
    uint32_t fhz // fixed point 16,16
) {
    // Assume f/fs < 1
    uint64_t t1 = (((uint64_t)fhz) << 32ULL) / US_SAMPLE_RATE_HZ;
    // Shift right until only 31 bits are used (slow)
    int32_t e = 0;
    while(t1 > ((1UL << 31UL) - 1UL)) {
        e++;
        t1 >>= 1;
    }
    pitch->fips = t1;
    pitch->eips = 32 - e;
}

// Not sure how accurate this is. Use with caution.
void us_set_wavelength_ms(
    UsPitch *pitch,
    uint16_t ms 
) {
    // // Assume f/fs < 1
    // uint64_t t1 = ((1000ULL << 32ULL) / __mul_instruction(ms, US_SAMPLE_RATE_HZ);
    // // Shift right until only 31 bits are used (slow)
    // int32_t e = 0;
    // while(t1 > ((1UL << 31UL) - 1UL)) {
    //     e++;
    //     t1 >>= 1;
    // }
    // pitch->fips = t1;
    // pitch->eips = 32 - e;
}
