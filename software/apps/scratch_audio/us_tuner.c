#include "us_tuner.h"
#include "us_tuner_notes.h"

void __not_in_flash_func(us_tuner_set_note)(
    UsTuner *tuner,
    uint32_t ni
){
    tuner->eips = us_bas[ni];
    tuner->fips = us_bae[ni];
}

uint32_t inline us_rotate_facc(
   UsTuner *tuner
) {
    const uint32_t eips = (uint32_t)tuner->eips;
    const uint32_t facc = tuner->facc + tuner->fips;
    const uint32_t hacc = facc >> eips;
    tuner->facc = facc - (hacc << eips);
    return hacc;
}

// Update for single sample
void __not_in_flash_func(us_tuner_rotate)(
    UsTuner *tuner
) {
    tuner->bang += us_rotate_facc(tuner);
}

// Update for single sample return true if wrapped
bool __not_in_flash_func(us_tuner_rotate_check_wrap)(
    UsTuner *tuner
) {
    const uint32_t bang_prev = tuner->bang;
    const uint32_t bang_next = bang_prev + us_rotate_facc(tuner);
    const bool wrap = bang_next < bang_prev;
    tuner->bang = bang_next;
    return wrap;
}