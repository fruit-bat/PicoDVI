#include "us_patch_1.h"
#include "us_debug.h"

void us_patch_1_init_config(UsPatch1Config* patch1_config) {
    US_DEBUG("US_PATCH: init config\n");

	us_adsr_config_init(&patch1_config->adsr_config);
    patch1_config->wave_func = us_wave_saw /* us_wave_not_square_lerp  us_wave_saw */;
}

static void init_data(void* data, void* config) {
    US_DEBUG("US_PATCH: init data\n");

    UsPatch1Data *patch1_data = (UsPatch1Data*)data;
    UsPatch1Config *patch1_config = (UsPatch1Config*)config;
    us_adsr_init(&patch1_data->adsr, &patch1_config->adsr_config);
}

static void note_on(void* data, void* config, uint32_t note, int32_t bend, uint32_t velocity) {
    US_DEBUG("US_PATCH: note on %ld, bend %ld, velocity %ld\n", note, bend, velocity);

    UsPatch1Data *patch1_data = (UsPatch1Data*)data;
    us_tuner_reset_phase(&patch1_data->tuner); // Set the phase to 0
    us_tuner_set_note(&patch1_data->tuner, note, bend);
    us_adsr_attack(&patch1_data->adsr, velocity);
}

static void note_release(void* data, void* config, uint32_t velocity) {
    US_DEBUG("US_PATCH: note release velocity %ld\n", velocity);

    UsPatch1Data *patch1_data = (UsPatch1Data*)data;
    us_adsr_release(&patch1_data->adsr);
}

static void bend(void* data, void* config, uint32_t note, int32_t bend) {
    US_DEBUG("US_PATCH: bend note %ld, bend %ld\n", note, bend);

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
    patch->init_data = init_data;
    patch->note_on = note_on;
    patch->note_release = note_release;
    patch->bend = bend;
    patch->update = update;
}
