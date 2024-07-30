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

static inline uint32_t us_midi_in_data(UsMidiIn *us_midi_in, uint32_t index) {
    return (uint32_t)us_midi_in->d[index];
}

static inline uint32_t us_midi_in_14bit_data(UsMidiIn *us_midi_in) {
    return us_midi_in_data(us_midi_in, 0) | (us_midi_in_data(us_midi_in, 1) << 7);
}

static inline void us_midi_in_status_message(UsMidiIn *us_midi_in){
    const uint32_t n = us_midi_in->sc;
    switch(us_midi_in->sm) {
        case 0: { // 1000nnnn	0kkkkkkk	0vvvvvvv	Note Off	n=channel* k=key # 0-127 (60=middle C) v=velocity (0-127)
            const uint32_t k = us_midi_in_data(us_midi_in, 0);
            const uint32_t v = us_midi_in_data(us_midi_in, 1);
            printf("Note off: n=%ld k=%ld v=%ld\n", n, k, v);

            break;
        }
        case 1: { // 1001nnnn	0kkkkkkk	0vvvvvvv	Note On	n=channel k=key # 0-127(60=middle C) v=velocity (0-127)
            const uint32_t k = us_midi_in_data(us_midi_in, 0);
            const uint32_t v = us_midi_in_data(us_midi_in, 1);
            printf("Note on: n=%ld k=%ld v=%ld\n", n, k, v);

            break;
        }
        case 6: { // 1110nnnn	0fffffff	0ccccccc	Pitch Bend	n=channel c=coarse f=fine (c+f = 14-bit resolution)
            const uint32_t b = us_midi_in_14bit_data(us_midi_in);
            printf("Pitch bend: n=%ld b=%ld\n", n, b);

            break;
        }
    }
}

void __not_in_flash_func(us_midi_in_update)(UsMidiIn *us_midi_in) {
    if (uart_is_readable(US_MIDI_UART_ID)) {
        const uint8_t k = uart_getc(US_MIDI_UART_ID);
        if (k != 248) printf("state %d data %d %8.8b\n", us_midi_in->state, k, k);        
        switch(us_midi_in->state) {
            case UsMidiInStatus: {
                if (k & 0b10000000) {
                    // Error in status message
                    printf("error in midi status message\n");
                    us_midi_in->state = UsMidiInIdle;
                }
                else {
                    us_midi_in->d[us_midi_in->di++] = k;
                    if (us_midi_in->di >= us_midi_in->dl) {
                        // We have a complete status message
                        if (us_midi_in->dl == 2) {
                            printf("ms %d %d %8.8b %8.8b\n",
                                us_midi_in->sm,
                                us_midi_in->sc,
                                us_midi_in->d[0],
                                us_midi_in->d[1]);
                        }
                        else {
                            printf("ms %d %d %8.8b\n",
                                us_midi_in->sm,
                                us_midi_in->sc,
                                us_midi_in->d[0]);
                        }
                        us_midi_in_status_message(us_midi_in);
                        us_midi_in->state = UsMidiInIdle;
                    }
                    break;
                }       
            }    
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
                    us_midi_in->state = UsMidiInStatus;
                }
                break;
            }    
            case UsMidiSysExec: {
                if ((k & 0b11111000) == 0b11111000) {
                    // System Real-Time Messages
                }
                else if (k == 0b11110111) {
                    // End of Sys Exec
                    us_midi_in->state = UsMidiInIdle;
                }
                break;
            }
            default: break;
        }
    }
}
