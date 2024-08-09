#pragma once
#include <pico/stdlib.h>
#include "us_patch.h"
#include "us_adsr.h"
#include "us_uint8_dlist.h"

#define US_NOT_A_NOTE 255
#define US_NOTE_COUNT 128

typedef struct {
    UsUint8DlistEntry links;
    uint8_t note;
    uint8_t status; // UsPatchState
} UsChannelPatchState;

typedef struct {

    // Output storage
    int32_t out_l;
    int32_t out_r;

    // Pitch bend is channel wide
    int32_t bend;

    // Channel id
    uint8_t id;

    // 0 <= Gain <= 128 // TODO Think about this one (0-127 coming from midi files)
    uint8_t gain;

    // 0 <= Pan <= 128 // TODO Think about this one (0-127 coming from midi files) 64 centred
    uint8_t pan;

    // Three lists are maintained for patch instances (voices) { off, on, released }
    UsUint8DlistAnchor patch_state_lists[UsPatchStateCount];

    // The patch interface
    UsPatch patch;

    // The number of patch instances
    uint32_t patch_count;

    // Array of patch data containing patch_count elements
    // This is for patch specific data
    void *patch_data;

    // Array of patch state containing patch_count elements
    // This is the state needed by the channel per patch instance
    UsChannelPatchState *patch_state;

    // Single shared patch config across all patch instances
    void *patch_config;

    // sizeof 1 element of patch_data
    size_t patch_data_size;

    // Callbacks from the patch to notify the channel of state changes
    UsPatchCallbacks patch_callbacks;

    // An index of note to patch instance
    uint8_t notes[US_NOTE_COUNT];

    void *callback_data;
    void (*callback_on)(void *data, uint8_t id);
    void (*callback_off)(void *data, uint8_t id);

} UsChannel;

// Patch callbacks
void us_channel_patch_cb_release(void *d, uint32_t id);
void us_channel_patch_cb_off(void *d, uint32_t id);

// API
void us_channel_init(
    UsChannel *channel,
    uint8_t id, 
    void* callback_data, 
    void (*on)(void *data, uint8_t id), 
    void (*off)(void *data, uint8_t id));

void us_channel_update(UsChannel* channel);
void us_channel_note_on(UsChannel* channel, uint32_t note, uint32_t velocity);
void us_channel_note_release(UsChannel* channel, uint32_t note, uint32_t velocity);
void us_channel_bend(UsChannel* channel, int32_t bend);

inline void us_channel_vol(UsChannel* channel, uint32_t vol) {
    channel->gain = vol;
}

inline void us_channel_pan(UsChannel* channel, uint32_t pan) {
    channel->pan = pan;
}

void us_channel_set_patch(
    UsChannel *channel,
    void (*init_patch)(UsPatch *patch),
    void * patch_config,
    void * patch_data,
    size_t patch_data_size,
    UsChannelPatchState *patch_state,
    uint32_t patch_count);
