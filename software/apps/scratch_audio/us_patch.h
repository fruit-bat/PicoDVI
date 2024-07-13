#pragma once
#include <pico/stdlib.h>

typedef struct {
    void (*on)(void *d, uint32_t id);
    void (*rel)(void *d, uint32_t id);
    void (*off)(void *d, uint32_t id);
} UsPatchCallbacks;

typedef struct {
    int32_t (*update)(void* data, UsPatchCallbacks* callbacks, uint32_t callback_id);


    void (*init)(void* data);
    void (*note_on)(void* data, uint32_t note, int32_t bend, uint32_t velocity);
    void (*note_off)(void* data, uint32_t velocity);
    void (*bend)(void* data, int32_t bend);
    bool (*is_off)(void* data);
} UsPatch;

