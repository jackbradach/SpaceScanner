/* File: neopixels_avr.c
 * Contains the low-level AVR specific routines needed to drive WS1282-type
 * programmable LEDs, aka "Neopixels."
 */
#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "config.h"
#include "led_gamma.h"
#include "neopixels.h"
#include "pixels.h"

/* Symbol timing constants, a hand-wavy approximation of what the WS2812B spits
 * out as the "reshaped" signal.  Values in ns.  The adjustments were
 * determined empirically @ 20mHz;  I reckon that some fudging will be
 * necessary for different F_CPUs.
 */

// #define T0H 450.0
// #define T1H 850.0
// #define TxL 800.0
// #define RES 7000.0

/* Symbol times in Microseconds*/
#define T0H  0.350
#define T1H  0.700
#define TxL  0.600
#define TRES 6.000


// TODO: rewrite this to send a byte at a time, using assembly maybe.
void neopixels_test_pattern(neopixels_t *chain);
void neopixels_color_send(neopixels_t *chain, uint8_t color);

/* Helper to pull the data line on a Neopixel chain high. */
inline void neopixels_data_high(neopixels_t *chain)
{
    *chain->port |= _BV(chain->pin);
}

/* Helper to pull the data line on a Neopixel chain low. */
inline void neopixels_data_low(neopixels_t *chain)
{
    *chain->port &= ~_BV(chain->pin);
}

void neopixels_color_send_gamma(neopixels_t *chain, uint8_t color)
{
    neopixels_color_send(chain, led_gamma_correct(color));
}

void neopixels_color_send(neopixels_t *chain, uint8_t color)
{
    neopixels_write_byte(chain->port, chain->pin, color);

}

/* Latches the data into the neopixel chain.  This is essentially dead time
 * and could be rewritten so that this sets a mutex using the RES time!
 */
void neopixels_latch(neopixels_t *chain)
{
    neopixels_data_low(chain);
    _delay_us(TRES);
}

void __attribute__((optimize("O3"))) neopixels_refresh(neopixels_t *chain)
{
    uint8_t mask = _BV(chain->pin);
    cli();
    for (uint8_t led_idx = 0; led_idx < chain->length; led_idx++) {
        for (uint8_t color_idx = 0; color_idx < PIXELS_CH_MAX; color_idx++) {
            uint8_t gc_color = led_gamma_correct(chain->color[led_idx][color_idx]);
            uint8_t sym = gc_color & 0x80;
            for (uint8_t bit = 0; bit < 8; bit++) {
                /* Rising edge of bit */
                PORTD |= mask;
                if (sym) {
                    _delay_us(T1H + T1H_ADJ);
                } else {
                    _delay_us(T0H + T0H_ADJ);
                }
                PORTD &= ~mask;
                gc_color <<= 1;
                sym = gc_color & 0x80;
                /* Skip delay on last bit; it's covered by the overhead
                 * of loading up the next byte to send.
                 */
                if (bit != 7) {
                    _delay_us(TxL + TxL_ADJ);
                }
            } // foreach bit
        } // foreach color (per LED)
    } // foreach LED
    sei();
    neopixels_latch(chain);
} // neopixels_refresh()



static volatile uint8_t *ddr_from_port(volatile uint8_t *port)
{
    if (0) {}
#if defined(PORTA)
    else if (&PORTA == port)
        return &DDRA;
#endif
#if defined(PORTB)
    else if (&PORTB == port)
        return &DDRB;
#endif
#if defined(PORTD)
    else if (&PORTD == port)
        return &DDRD;
#endif
    else
        return NULL;
}

void neopixels_init_chain(neopixels_t *chain, volatile uint8_t *port, uint8_t pin, uint8_t length)
{
        volatile uint8_t *ddr = ddr_from_port(port);
        chain->port = port;
        chain->pin = pin;

        *chain->port &= ~_BV(chain->pin);
        *ddr |= _BV(pin);

        chain->length = length;

        /* Allocate enough memory to store each pixel's color data. */
        chain->color = calloc(length, sizeof(uint8_t *));
        for (int led_idx = 0; led_idx < length; led_idx++) {
            chain->color[led_idx] = calloc(PIXELS_CH_MAX, sizeof(uint8_t));
        }
        neopixels_off(chain);
}

void neopixels_off(neopixels_t *chain)
{
    for (int led_idx = 0; led_idx < chain->length; led_idx++) {
        for (int color_idx = 0; color_idx < 3; color_idx++ ) {
            chain->color[led_idx][color_idx] = 0x0;
        }
    }
    neopixels_refresh(chain);
}

/* Bit-bangs the WS2812 protocol to the pixels. */

// void __attribute__((optimize("O3"))) neopixels_write_byte(volatile uint8_t *port, uint8_t pin, uint8_t color)
// {
//     const uint8_t mask_high = _BV(pin);
//     const uint8_t mask_low = ~_BV(pin);
//     uint8_t sym;
//     sym = color & 0x80;
//     for (int bit = 0; bit < 8; bit++) {
//         /* Rising edge of bit */
//         PORTD |= mask_high;
//         if (sym) {
//             _delay_us(T1H + T1H_ADJ);
//         } else {
//             _delay_us(T0H + T0H_ADJ);
//         }
//         PORTD &= mask_low;

//         color <<= 1;
//         sym = color & 0x80;

//         if (bit != 7) {
//             _delay_us(TxL + TxL_ADJ);
//         }
//     }
// }


#define PIXELS_CONFIG_TEST_PATTERN
#ifdef PIXELS_CONFIG_TEST_PATTERN
/* Generates a timing test pattern, suitable for setting the TxY_ADJ parameters
 * to compensate for different execution times.  Use a logic analyzer to
 * measure the 0b01010101 and latch pulse times.  Change the TxY_ADJ parameters
 * to compensate.  Not a great solution but it works for now.
 */
void neopixels_test_pattern(neopixels_t *chain)
{
          neopixels_write_byte(chain->port, chain->pin, 0x55);

          /* The data line pulses before and after the latch are
           * to make the duration of the latch easier to pick
           * out on the logic analyzer; otherwise it just looks
           * like I left the line low for a bit!
           */
          neopixels_data_high(chain);
          neopixels_latch(chain);
          neopixels_data_high(chain);
          neopixels_data_low(chain);
}
#endif