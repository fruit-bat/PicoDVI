#pragma once
#include "pico/stdlib.h"

enum UsPmCommands {
    SynCmdPPQ,
    SynCmdTempo,
    SynCmdOn,
    SynCmdOff,
    SynCmdTime,
    SynCmdEnd
};

typedef uint8_t * UsPmCursor;

typedef struct {
    uint8_t voice;
    uint8_t key;
    uint8_t velocity;
} UsPmNoteOn;

typedef struct {
    uint8_t voice;
    uint8_t velocity;
} UsPmNoteOff;

typedef struct {
    uint16_t ticks;
} UsPmTickDelta;

typedef struct {
    uint16_t ppq;
} UsPmPpq;

typedef struct {
    uint32_t tempo;
} UsPmTempo;

typedef struct {
    uint8_t type;
    union
    {
        UsPmNoteOn note_on;
        UsPmNoteOff note_off;
        UsPmTickDelta tick_delta;
        UsPmPpq ppq;
        UsPmTempo tempo;
    } data;
    
} UsPmMessage;

