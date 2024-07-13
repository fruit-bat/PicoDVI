#include "us_patch_1.h"

static void init_config(void* config) {
    UsPatch1Config *patch1Config = (UsPatch1Config*)config;

	us_adsr_config_init(&patch1Config->adsr_config);
    patch1Config->wave_func = us_wave_saw;
}

static void init(void* data, void* config) {
    UsPatch1 *patch1 = (UsPatch1*)data;
    UsPatch1Config *patch1Config = (UsPatch1Config*)config;

    us_adsr_init(&patch1->adsr, &patch1Config->adsr_config);    
}

static void note_on(void* data, uint32_t note, int32_t bend, uint32_t velocity) {
    UsPatch1 *patch1 = (UsPatch1*)data;

    us_tuner_reset_phase(&patch1->tuner); // Set the phase to 0
    us_tuner_set_note(&patch1->tuner, note, bend);
    us_adsr_attack(&patch1->adsr, velocity);
}

static void note_off(void* data, uint32_t velocity) {
    UsPatch1 *patch1 = (UsPatch1*)data;

    us_adsr_release(&patch1->adsr);
}

static void bend(void* data, uint32_t note, int32_t bend) {
    UsPatch1 *patch1 = (UsPatch1*)data;
    us_tuner_set_note(&patch1->tuner, note, bend);
}

static int32_t update(void* data, void* config, UsPatchCallbacks* callbacks, void *callback_data, uint32_t callback_id) {
    UsPatch1 *patch1 = (UsPatch1*)data;
    UsPatch1Config *patch1Config = (UsPatch1Config*)config;

    const int32_t adsr = us_adsr_update(
        &patch1->adsr,
        callbacks,
        callback_data,
        callback_id);

    if (us_adsr_is_off(&patch1->adsr)) {
        return 0;
    }
    else {
        us_tuner_rotate(&patch1->tuner);
        return __mul_instruction(patch1Config->wave_func(patch1->tuner.bang), adsr) >> 16;
    }
    return 0;
}

void us_patch_1_apply(UsPatch *patch) {
    patch->init_config = init_config;
    patch->init = init;
    patch->note_on = note_on;
    patch->note_off = note_off;
    patch->bend = bend;
    patch->update = update;
}

UsPatch patch1 = {
    .init_config = init_config,
    .init = init,
    .note_on = note_on,
    .note_off = note_off,
    .bend = bend,
    .update = update
};
