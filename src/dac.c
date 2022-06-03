#include <stdbool.h>
#include <stdio.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#include "config.h"
#include "dac.h"
#include "dbg.h"

#define DAC_PORT CONFIG_DAC_PORT
#define DAC_BIT_A CONFIG_DAC_BIT_A 
#define DAC_BIT_B CONFIG_DAC_BIT_B

uint16_t (*dac_sampler_cb)() = NULL;

void dac_init() {
    // Fast PWM
    // TODO - 2022/06/01 - jbradach - fiddle with this to get more bits, maybe use phase-correct PWM?
    OCR1A = 0;
    TIMSK1 = 0;
    PORTB = 0;
    TCCR1A = _BV(COM1A1) |  _BV(WGM11) | _BV(WGM10);
    TCCR1B = _BV(WGM12);

    // CTC @ 8000 Hz (16000000/((249+1)*8))
    // 4000 Hz (16000000/((124+1)*32))
    TCNT2 = 0;
    OCR2A = 124;
    TCCR2A = _BV(WGM21);

    TIMSK2 |= _BV(OCIE2A);
    DDRB |= (DAC_BIT_A | DAC_BIT_B); // turn on outputs
}

/* Binds a callback to a function that can update the OCR
 * register during the sampling ISR.
 */
void dac_set_sampler_cb(uint16_t (sampler_cb())) {
    dac_sampler_cb = sampler_cb;
}

void dac_start() {
    TCCR1B |= _BV(CS10);
    TCCR2B |= _BV(CS21) | _BV(CS20);
    // printf("DAC STARTED!\n");
}

void dac_stop() {
    TCCR1B &= ~_BV(CS10);
    TCCR2B &= ~(_BV(CS21) | _BV(CS20));
    OCR1A = 0;
}

ISR(TIMER2_COMPA_vect) {
    if (NULL != dac_sampler_cb) {
        uint16_t v = dac_sampler_cb();
        OCR1A = v;
    } else {
        OCR1A = 0;
    }
}

