#pragma once
#include <pico/stdlib.h>
#include "us_patch.h"
#include "us_adsr.h"
#include "us_uint8_dlist.h"

#define US_MAX_PATCH_PER_CHANNEL 32
#define US_NOT_A_NOTE 255

typedef struct {
    UsUint8DlistEntry links;
    uint8_t note;
    uint8_t status; // UsPatchState
} UsChannelPatchState;

typedef struct {
    int32_t bend;
    uint32_t gain;                           // 0 <= Gain <= 256 // TODO Think about this one (0-127 coming from midi files)
    UsUint8DlistAnchor patch_state_lists[UsPatchStateCount];
    UsPatch patch;
    uint32_t patch_count;
    void *patch_data;   // Array of patch data containing patch_count elements
    UsChannelPatchState *patch_state;  // Array of patch state containing patch_count elements
    void *patch_config; // Single shared patch config structure
    size_t patch_data_size; // sizeof 1 element of patch_data
    UsPatchCallbacks patch_callbacks;
} UsChannel;

// Patch callbacks
void us_channel_patch_cb_release(void *d, uint32_t id);
void us_channel_patch_cb_off(void *d, uint32_t id);

// API
void us_channel_init(UsChannel *channel);
int32_t us_channel_update(UsChannel* channel);
void us_channel_note_on(UsChannel* channel, uint32_t note, uint32_t velocity);
void us_channel_note_off(UsChannel* channel, uint32_t note, uint32_t velocity);
void us_channel_bend(UsChannel* channel, int32_t bend);

void us_channel_set_patch(
    UsChannel *channel,
    void (*init_patch)(UsPatch *patch), 
    void * patch_config,
    void * patch_data,
    size_t patch_data_size,
    UsChannelPatchState *patch_state,
    uint32_t patch_count);