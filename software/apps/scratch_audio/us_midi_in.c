#include "us_midi_in.h"
#include "us_midi_uart.h"
#include <stdio.h>

void us_midi_in_init(UsMidiIn *us_midi_in) {
    us_midi_in->state = UsMidiInIdle;
}

static uint8_t message_data_lengths[8] = {
    2, // 1000nnnn	0kkkkkkk    0vvvvvvv	Note Off event.
    2, // 1001nnnn	0kkkkkkk    0vvvvvvv	Note On event.
    2, // 1010nnnn	0kkkkkkk    0vvvvvvv	Polyphonic Key Pressure (Aftertouch).
    2, // 1011nnnn	0ccccccc    0vvvvvvv	Control Change.
    1, // 1100nnnn	0ppppppp	            Program Change. 
    1, // 1101nnnn	0vvvvvvv            	Channel Pressure (After-touch). 
    2, // 1110nnnn	0lllllll    0mmmmmmm	Pitch Bend Change. 
};

void __not_in_flash_func(us_midi_in_update)(UsMidiIn *us_midi_in) {
    if (uart_is_readable(US_MIDI_UART_ID)) {
        const uint8_t k = uart_getc(US_MIDI_UART_ID);
        if (k != 248) printf("Received %d %b\n", k, k);        
        switch(us_midi_in->state) {
            case UsMidiInIdle: {
                if ((k & 0b11111000) == 0b11111000) {
                    // System Real-Time Messages
                }
                else if ((k & 0b11111000) == 0b11111000) {
                    // System Common Messages
                }
                else if (k & 0b10000000) {
                    const uint8_t sm = (k >> 4) & 0b111;
                    const uint8_t sc = k & 0b1111;
                    us_midi_in->sm = sm;
                    us_midi_in->sc = sc;
                    us_midi_in->di = 0;
                    us_midi_in->dl = message_data_lengths[sm];

                }
            }
            case UsMidiInSatus: {

            }        
            case UsMidiSysExec: {
                if ((k & 0b11111000) == 0b11111000) {
                    // System Real-Time Messages
                }
                else if (k == 0b11110111) {
                    // End of Sys Exec
                    us_midi_in->state = UsMidiInIdle;
                }
            }
            default: break;
        }
    }
}
