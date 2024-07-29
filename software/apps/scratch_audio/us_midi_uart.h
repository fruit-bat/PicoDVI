#pragma once

#define US_MIDI_UART_ID uart1
#define US_MIDI_BAUD_RATE 31250
#define US_MIDI_DATA_BITS 8
#define US_MIDI_STOP_BITS 1
#define US_MIDI_PARITY    UART_PARITY_NONE

// We are using pins 0 and 1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.
#define US_MIDI_UART_TX_PIN 4
#define US_MIDI_UART_RX_PIN 5

void us_midi_uart_init();
void us_midi_uart_loop_test();