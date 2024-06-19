#pragma once
//
// MicroSynth PackedMidi 
//
#include "pico/stdlib.h"
#include "us_tuner.h"

enum UsPmCommands {
    SynCmdPPQ = 0,
    SynCmdTempo,
    SynCmdOn,
    SynCmdOff,
    SynCmdTime,
    SynCmdEnd
};

enum UsPmCommandLen {
    SynCmdPPQLen = 3,
    SynCmdTempoLen = 4,
    SynCmdOnLen = 4,
    SynCmdOffLen = 3,
    SynCmdTimeLen = 3,
    SynCmdEndLen = 1
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

typedef struct {
    UsTuner clock;      // Runs as micro seconds * parts per quater beat (us*PPQ)
    uint32_t tempo;     // Micro seconds per quarter beat (us)
    UsPmCursor cursor;  // Cursor into packed midi file
    int32_t ticks;      // Ticks to wait
} UsPmSequencer;