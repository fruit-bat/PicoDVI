#pragma once
#include "pico/stdlib.h"
#include "us_channels.h"

enum UsMidiInState {
    UsMidiInIdle = 0,
    UsMidiInStatus,
    UsMidiSysExec
};

typedef struct  {
    uint8_t state;          // Interpreter state
    uint8_t sm;             // status message
    uint8_t sc;             // status channel
    uint8_t d[2];           // data bytes
    uint8_t dl;             // data length
    uint8_t di;             // data index
    UsChannels *channels;   // channels
} UsMidiIn;

void us_midi_in_init(UsMidiIn *us_midi_in, UsChannels *channels);

void us_midi_in_update(UsMidiIn *us_midi_in);
