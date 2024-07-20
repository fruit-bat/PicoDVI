#pragma once
#include <pico/stdlib.h>
#include "us_uint8_dlist.h"

enum UsPatchState {
    UsPatchStateOff = 0,
    UsPatchStateOn,
    UsPatchStateRelease,
    UsPatchStateCount       // The number of states
};

typedef struct {
    void (*release)(void *d, uint32_t id);
    void (*off)(void *d, uint32_t id);
} UsPatchCallbacks;

typedef struct {
    void (*init_config)(void* config);
    void (*init_data)(void* data, void* config);
    void (*note_on)(void* data, uint32_t note, int32_t bend, uint32_t velocity);
    void (*note_off)(void* data, uint32_t velocity);
    void (*bend)(void* data, uint32_t note, int32_t bend);
    int32_t (*update)(void* data, void* config, UsPatchCallbacks* callbacks, void *callback_data, uint32_t callback_id);
} UsPatch;
