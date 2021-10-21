#ifndef LED_GAMMA_H
#define LED_GAMMA_H

#include <avr/pgmspace.h>

extern const uint8_t led_gamma[];

/* Looks up val in a gamma correction table and returns the result. */
static inline uint8_t led_gamma_correct(uint8_t val)
{
    return pgm_read_byte(&led_gamma[val]);
}

#endif
