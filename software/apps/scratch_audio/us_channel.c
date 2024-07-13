#include "us_channel.h"

void us_channel_init(UsChannel *channel) {
    channel->bend = 0;
    channel->gain = 256; // Full volume
    channel->patch_count = 0; // TODO pass in patch data etc.

    const uint32_t patch_count = channel->patch_count;
    const UsPatch *patch = &channel->patch;
    uint8_t* d = channel->patch_data;
    uint32_t s = channel->patch_data_size;
    for(int32_t i = 0; i < patch_count; ++i) {
        patch->init(d);
        d += s;
    }
    for(int32_t i = 0; i < US_MAX_PATCH_PER_CHANNEL; ++i) {
        channel->note[i] = US_NOT_A_NOTE;
    }
}

void us_channel_note_on(UsChannel* channel, uint32_t note, uint32_t velocity) {
    const UsPatch *patch = &channel->patch;
    uint8_t* d = channel->patch_data;
    uint32_t s = channel->patch_data_size;
    for (uint32_t i = 0; i < channel->patch_count; ++i) {        
        if (patch->is_off(d)) {
            channel->note[i] = (uint8_t)note;
            patch->note_on(d, note, channel->bend, velocity);
        }
        d += s;
    }
}

void us_channel_note_off(UsChannel* channel, uint32_t note, uint32_t velocity) {
    const UsPatch *patch = &channel->patch;
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

int32_t __not_in_flash_func(us_channel_update)(UsChannel *channel) {
    int32_t out = 0;
    const uint32_t patch_count = channel->patch_count;
    const UsPatch *patch = &channel->patch;
    uint8_t* d = channel->patch_data;
    uint32_t s = channel->patch_data_size;

    UsPatchCallbacks callbacks;
    callbacks.on = 0; // TODO
    callbacks.off = 0;
    callbacks.rel = 0;

    for(int32_t i = 0; i < patch_count; ++i) {
        out += patch->update(d, &callbacks, i);
        d += s;
    }
    
    if (out > 32767) return 32767;
    if (out < -32768) return -32768;
    return out;
}
