#include "us_patch_1.h"

void init_config(void* config) {
    UsPatch1Config *patch1_config = (UsPatch1Config*)config;
	us_adsr_config_init(&patch1_config->adsr_config);
    patch1_config->wave_func = us_wave_saw;
}

static void init_data(void* data, void* config) {
    UsPatch1Data *patch1_data = (UsPatch1Data*)data;
    UsPatch1Config *patch1_config = (UsPatch1Config*)config;
    us_adsr_init(&patch1_data->adsr, &patch1_config->adsr_config);    
}

static void note_on(void* data, uint32_t note, int32_t bend, uint32_t velocity) {
    UsPatch1Data *patch1_data = (UsPatch1Data*)data;
    us_tuner_reset_phase(&patch1_data->tuner); // Set the phase to 0
    us_tuner_set_note(&patch1_data->tuner, note, bend);
    us_adsr_attack(&patch1_data->adsr, velocity);
}

static void note_off(void* data, uint32_t velocity) {
    UsPatch1Data *patch1_data = (UsPatch1Data*)data;
    us_adsr_release(&patch1_data->adsr);
}

static void bend(void* data, uint32_t note, int32_t bend) {
    UsPatch1Data *patch1_data = (UsPatch1Data*)data;
    us_tuner_set_note(&patch1_data->tuner, note, bend);
}

static int32_t update(void* data, void* config, UsPatchCallbacks* callbacks, void *callback_data, uint32_t callback_id) {
    UsPatch1Data *patch1_data = (UsPatch1Data*)data;
    UsPatch1Config *patch1_config = (UsPatch1Config*)config;

    const int32_t adsr = us_adsr_update(
        &patch1_data->adsr,
        callbacks,
        callback_data,
        callback_id);

    if (us_adsr_is_off(&patch1_data->adsr)) {
        return 0;
    }
    else {
        us_tuner_rotate(&patch1_data->tuner);
        return __mul_instruction(patch1_config->wave_func(patch1_data->tuner.bang), adsr) >> 16;
    }
    return 0;
}

void us_patch_1_apply(UsPatch *patch) {
    patch->init_config = init_config;
    patch->init_data = init_data;
    patch->note_on = note_on;
    patch->note_off = note_off;
    patch->bend = bend;
    patch->update = update;
}
