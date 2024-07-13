#pragma once
#include <pico/stdlib.h>
#include "us_patch.h"

#define US_MAX_PATCH_PER_CHANNEL 32
#define US_NOT_A_NOTE 255

typedef struct  {
    int32_t bend;
    uint32_t gain;                           // 0 <= Gain <= 256 // TODO Think about this one (0-127 coming from midi files)
    uint8_t note[US_MAX_PATCH_PER_CHANNEL];  // Remeber the notes that was played so we can stop them
    UsPatch patch;
    uint32_t patch_count;
    void *patch_data;
    size_t patch_data_size; // sizeof 1 element
} UsChannel;

void us_channel_init(UsChannel *channel);

int32_t us_channel_update(UsChannel* channel);

void us_channel_note_on(UsChannel* channel, uint32_t note, uint32_t velocity);

void us_channel_note_off(UsChannel* channel, uint32_t note, uint32_t velocity);

void us_channel_bend(UsChannel* channel, int32_t bend);

