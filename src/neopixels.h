#ifndef _NEOPIXELS_H_
#define _NEOPIXELS_H_

#include <stdint.h>
#include "pixels.h"

/* Describes a chain of neopixels */
typedef struct neopixels_s {
    /* How many Neopixels are in the chain.
     * Limit 255, until I do a project big
     * enough to need a uint16_t.
     */
    uint8_t length;

    /* Each Neopixel has a 24-bit color value. */
    uint8_t **color;
  
    // TODO - jbradach - move to its own structure
    volatile uint8_t *port;
    uint8_t pin;
    uint8_t mask;
    void (*refresh)(struct neopixels_s *np);

} neopixels_t;

typedef struct neopixel_io_s {
    void *refresh;
    void *write;
    void *latch;

} neopixels_io_t;

void neopixels_off(neopixels_t *chain);
void neopixels_set_color_all(neopixels_t *chain, pixels_color_t *color);
void neopixels_refresh(neopixels_t *chain);

void neopixels_color_send_gamma(neopixels_t *chain, uint8_t color);
void neopixels_color_send(neopixels_t *chain, uint8_t color);

/* AVR Specific */
void neopixels_write_byte(volatile uint8_t *port, uint8_t pin, uint8_t color);
void neopixels_init_chain(neopixels_t *chain, volatile uint8_t *port, uint8_t pin, uint8_t length);
void neopixels_latch(neopixels_t *chain);

#endif