#include <stdint.h>

#include <avr/interrupt.h>
#include <avr/io.h>

#include "timer.h"

static uint32_t ticks;
#define MS_PER_TICK 16

void timer_init() {
    /* Set up the WDT as a sloppy tick source */
    ticks = 0;
    WDTCSR = _BV(WDIE);
}

uint32_t get_ticks() {
    return ticks;
}

uint32_t get_ticks_ms() {
    return ticks * MS_PER_TICK;
}

/* We're using the WDT as a sloppy timer. */
ISR(WDT_vect) {
    ticks++;
}