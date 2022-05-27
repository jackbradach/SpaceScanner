#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "config.h"
#include "dac.h"

#define DAC_PORT CONFIG_DAC_PORT
#define DAC_BIT_A CONFIG_DAC_BIT_A 
#define DAC_BIT_B CONFIG_DAC_BIT_B

#define PWM_FREQ 0x00FF // pwm frequency - see table
#define PWM_MODE 0 // Fast (1) or Phase Correct (0)
#define PWM_QTY 2 // number of pwms, either 1 or 2

static uint8_t audio_data;

#define SINE_LENGTH 256
extern const uint8_t sinewave[SINE_LENGTH];


uint8_t sine_idx;

const uint16_t A4_FREQ = 440;
#define TIMER1_PRESCALER 8
#define A4 (F_CPU / (A4_FREQ * TIMER1_PRESCALER) - 1)

void dac_init() {
    audio_data = 0;
    sine_idx = 0;
    // setup PWM
 
    TCCR1A = _BV(COM1A1) |  _BV(WGM10);
    //   | _BV(WGM11) | _BV(WGM10);
    TCCR1B =  _BV(CS10) | _BV(CS10) | _BV(WGM12);
    TIMSK1 = 0;
    // TCCR1B = _BV(WGM12) | _BV(CS10);

    // TCCR1A = (((PWM_QTY - 1) << 5) | 0x80 | (PWM_MODE << 1)); // 
    // TCCR1B = ((PWM_MODE << 3) | 0x11); // ck/1
    // // TIMSK1 = 0x20; // interrupt on capture interrupt
    // ICR1H = (PWM_FREQ >> 8);
    // ICR1L = (PWM_FREQ & 0xff);
    PORTB = 0;
    DDRB |= (DAC_BIT_A | DAC_BIT_B); // turn on outputs


    // XXX disable tone:
    // TCCR1B = 0;

    OCR1A = 77;
    // OCR1A = 0;

    TCNT2 = 0;
    // 8000 Hz (16000000/((249+1)*8))
    OCR2A = 249;
    // // CTC
    TCCR2A = _BV(WGM21);
    // // Prescaler 8
    TCCR2B = _BV(CS21);
    // // Output Compare Match A Interrupt Enable
    TIMSK2 |= _BV(OCIE2A);
}

void dac_set_value(uint16_t v);



ISR(TIMER2_COMPA_vect) {
    uint8_t v = pgm_read_byte(&sinewave[sine_idx++]);
    // sine_idx++;
    // OCR1A = v;
    // audio_data = 0;
// }
}

const uint8_t  sinewave[] PROGMEM = {
    0x80,0x83,0x86,0x89,0x8c,0x8f,0x92,0x95,0x98,0x9c,0x9f,0xa2,0xa5,0xa8,0xab,0xae,
    0xb0,0xb3,0xb6,0xb9,0xbc,0xbf,0xc1,0xc4,0xc7,0xc9,0xcc,0xce,0xd1,0xd3,0xd5,0xd8,
    0xda,0xdc,0xde,0xe0,0xe2,0xe4,0xe6,0xe8,0xea,0xec,0xed,0xef,0xf0,0xf2,0xf3,0xf5,
    0xf6,0xf7,0xf8,0xf9,0xfa,0xfb,0xfc,0xfc,0xfd,0xfe,0xfe,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xfe,0xfe,0xfd,0xfc,0xfc,0xfb,0xfa,0xf9,0xf8,0xf7,
    0xf6,0xf5,0xf3,0xf2,0xf0,0xef,0xed,0xec,0xea,0xe8,0xe6,0xe4,0xe2,0xe0,0xde,0xdc,
    0xda,0xd8,0xd5,0xd3,0xd1,0xce,0xcc,0xc9,0xc7,0xc4,0xc1,0xbf,0xbc,0xb9,0xb6,0xb3,
    0xb0,0xae,0xab,0xa8,0xa5,0xa2,0x9f,0x9c,0x98,0x95,0x92,0x8f,0x8c,0x89,0x86,0x83,
    0x80,0x7c,0x79,0x76,0x73,0x70,0x6d,0x6a,0x67,0x63,0x60,0x5d,0x5a,0x57,0x54,0x51,
    0x4f,0x4c,0x49,0x46,0x43,0x40,0x3e,0x3b,0x38,0x36,0x33,0x31,0x2e,0x2c,0x2a,0x27,
    0x25,0x23,0x21,0x1f,0x1d,0x1b,0x19,0x17,0x15,0x13,0x12,0x10,0x0f,0x0d,0x0c,0x0a,
    0x09,0x08,0x07,0x06,0x05,0x04,0x03,0x03,0x02,0x01,0x01,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x02,0x03,0x03,0x04,0x05,0x06,0x07,0x08,
    0x09,0x0a,0x0c,0x0d,0x0f,0x10,0x12,0x13,0x15,0x17,0x19,0x1b,0x1d,0x1f,0x21,0x23,
    0x25,0x27,0x2a,0x2c,0x2e,0x31,0x33,0x36,0x38,0x3b,0x3e,0x40,0x43,0x46,0x49,0x4c,
    0x4f,0x51,0x54,0x57,0x5a,0x5d,0x60,0x63,0x67,0x6a,0x6d,0x70,0x73,0x76,0x79,0x7c
};
