#include "us_midi_uart.h"
#include <stdio.h>
#include <stdlib.h>
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "hardware/gpio.h"

static void __not_in_flash_func(us_midi_uart_irq_rx)() {

}

void us_midi_uart_init() {
    // Set up our UART with a basic baud rate.
    uart_init(US_MIDI_UART_ID, 2400);

    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(US_MIDI_UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(US_MIDI_UART_RX_PIN, GPIO_FUNC_UART);

    // Actually, we want a different speed
    // The call will return the actual baud rate selected, which will be as close as
    // possible to that requested
    int __unused actual = uart_set_baudrate(US_MIDI_UART_ID, US_MIDI_BAUD_RATE);

    // Set UART flow control CTS/RTS, we don't want these, so turn them off
    uart_set_hw_flow(US_MIDI_UART_ID, false, false);

    // Set our data format
    uart_set_format(US_MIDI_UART_ID, US_MIDI_DATA_BITS, US_MIDI_STOP_BITS, US_MIDI_PARITY);
/*
    // Set up a RX interrupt
    // We need to set up the handler first
    // Select correct interrupt for the UART we are using
    int UART_IRQ = US_MIDI_UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;

    // And set up and enable the interrupt handlers
    irq_set_exclusive_handler(UART_IRQ, us_midi_uart_irq_rx);
    irq_set_enabled(UART_IRQ, true);

    // Now enable the UART to send interrupts - RX only
    uart_set_irq_enables(US_MIDI_UART_ID, true, false);
    */
}
void us_midi_uart_loop_test() {
    static char ch = 0;
    if (uart_is_writable(US_MIDI_UART_ID)) {
//        printf("Sending %d\n", ch);
        uart_putc(US_MIDI_UART_ID, ch++);        
    }
    while (uart_is_readable(US_MIDI_UART_ID)) {
        uint8_t k = uart_getc(US_MIDI_UART_ID);
        if (k != 248) printf("Received %d\n", k);
    }    
}
