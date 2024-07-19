#include "us_channel.h"

void us_channel_init(UsChannel *channel) {
    channel->bend = 0;
    channel->gain = 256; // Full volume
    channel->patch_count = 0;
    channel->patch_callbacks.off = us_channel_patch_cb_off;
    channel->patch_callbacks.release = us_channel_patch_cb_release;

    for(uint32_t i = 0; i < US_MAX_PATCH_PER_CHANNEL; ++i) {
        channel->note[i] = US_NOT_A_NOTE;
    }
}

void us_channel_set_patch(
    UsChannel *channel,
    void (*init_patch)(UsPatch *patch), 
    void * patch_config,
    void * patch_data, 
    size_t patch_data_size, 
    uint32_t patch_count) {

    init_patch(&channel->patch);
    channel->patch_data = patch_data;
    channel->patch_data_size = patch_data_size;
    channel->patch_count = patch_count;
    channel->patch_config = patch_config;

    channel->patch.init_config(patch_config);

    uint8_t* data = patch_data;

    for(int32_t i = 0; i < patch_count; ++i) {
        channel->patch.init(data, patch_config);
        data += patch_data_size;
    }
}

void us_channel_note_on(UsChannel* channel, uint32_t note, uint32_t velocity) {
    const UsPatch *patch = &channel->patch;
    if (patch) {
        uint8_t* d = channel->patch_data;
        uint32_t s = channel->patch_data_size;
        for (uint32_t i = 0; i < channel->patch_count; ++i) {        
    //      if (patch->is_off(d)) {
                channel->note[i] = (uint8_t)note;
                patch->note_on(d, note, channel->bend, velocity);
        //    }
            d += s;
        }
    }
}

void us_channel_note_off(UsChannel* channel, uint32_t note, uint32_t velocity) {
    const UsPatch *patch = &channel->patch;
    if (patch) {
        uint8_t* d = channel->patch_data;
        uint32_t s = channel->patch_data_size;
        for (uint32_t i = 0; i < channel->patch_count; ++i) {        
            if (channel->note[i] == (uint8_t)note) {
                patch->note_off(d, velocity);
                channel->note[i] = US_NOT_A_NOTE;
            }
            d += s;
        }
    }
}

void __not_in_flash_func(us_channel_bend)(UsChannel* channel, int32_t bend) {
    const UsPatch *patch = &channel->patch;
    if (patch) {
        // TODO
    }
}

int32_t __not_in_flash_func(us_channel_update)(UsChannel *channel) {
    int32_t out = 0;
    const UsPatch *patch = &channel->patch;
    if (patch) {
        const uint32_t patch_count = channel->patch_count;
        uint8_t* data = channel->patch_data;
        uint32_t s = channel->patch_data_size;
        UsPatchCallbacks *patch_callbacks = &channel->patch_callbacks;
        void* config = channel->patch_config;

        for(int32_t i = 0; i < patch_count; ++i) {
            out += patch->update(
                data,            // The data the patch needs to function
                config,          // The config common to all voices with this patch
                patch_callbacks, // Callbacks so the patch can report its state back to the channel
                channel,         // The channel data to be used in the patch callbacks
                i                // The patch index for use in the callback
            );
            data += s;
        }

        if (out > 32767) return 32767;
        if (out < -32768) return -32768;
    }
    return out;
}

void __not_in_flash_func(us_channel_patch_cb_release)(void *d, uint32_t id) {
    UsChannel *channel = (UsChannel *)d;
    // TODO the voice has been released and could now be re-allocated
}

void __not_in_flash_func(us_channel_patch_cb_off)(void *d, uint32_t id) {
    UsChannel *channel = (UsChannel *)d;
    // TODO the voice is now off and does not need to be updated
}
