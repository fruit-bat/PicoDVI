#pragma once
#include <pico/stdlib.h>

typedef struct {
    void *data;
    void (*init)(void* data);
    int32_t (*update)(void* data);
    void (*note_on)(void* data, uint32_t note, int32_t bend, uint32_t velocity);
    void (*note_off)(void* data, uint32_t velocity);
    void (*bend)(void* data, int32_t bend);
    bool (*is_off)(void* data); // Does adsr reisde in this or outside it?
} UsPatch;

void us_patch_init(UsPatch* patch);
