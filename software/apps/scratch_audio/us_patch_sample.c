#include "us_patch_sample.h"
#include "us_debug.h"

void us_patch_sample_init_config(UsPatchSampleConfig* patch_config, const int16_t *samples, uint32_t sample_count) {
    US_DEBUG("US_PATCH_SAMPLE: init config\n");

    patch_config->samples = samples;
    patch_config->sample_count = sample_count;
}

static void init_data(void* data, void* config) {
    US_DEBUG("US_PATCH_SAMPLE: init data\n");

    UsPatchSampleData *patch_data = (UsPatchSampleData*)data;
    UsPatchSampleConfig *patch_config = (UsPatchSampleConfig*)config;
    patch_data->pos = patch_config->sample_count;
}

static void note_on(void* data, void* config, uint32_t note, int32_t bend, uint32_t velocity) {
    US_DEBUG("US_PATCH_SAMPLE: note on %ld, bend %ld, velocity %ld\n", note, bend, velocity);

    UsPatchSampleData *patch_data = (UsPatchSampleData*)data;
    patch_data->pos = 0;
}

static void note_release(void* data, void* config, uint32_t velocity) {
    US_DEBUG("US_PATCH_SAMPLE: note release velocity %ld\n", velocity);

}

static void bend(void* data, void* config, uint32_t note, int32_t bend) {
    US_DEBUG("US_PATCH_SAMPLE: bend note %ld, bend %ld\n", note, bend);
}

static int32_t update(void* data, void* config, UsPatchCallbacks* callbacks, void *callback_data, uint32_t callback_id) {
    UsPatchSampleData *patch_data = (UsPatchSampleData*)data;
    UsPatchSampleConfig *patch_config = (UsPatchSampleConfig*)config;

    if (patch_data->pos >= patch_config->sample_count) {
        if (callbacks) callbacks->off(callback_data, callback_id);
        return 0;
    }

    return patch_config->samples[patch_data->pos++];
}

void us_patch_sample_apply(UsPatch *patch) {
    patch->init_data = init_data;
    patch->note_on = note_on;
    patch->note_release = note_release;
    patch->bend = bend;
    patch->update = update;
}
