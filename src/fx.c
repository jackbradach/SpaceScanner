#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <avr/pgmspace.h>
#include <util/delay.h>

#include "config.h"
#include "dac.h"
#include "dbg.h"
#include "fx.h"


#define SINE_LENGTH 256
extern const uint8_t sinewave[SINE_LENGTH];


#define NVOICES 3
typedef struct {
    uint16_t acc;
    uint16_t freq;

} voices_t;

volatile uint8_t sine_idx;

const uint16_t A4_FREQ = 440;
#define TIMER1_PRESCALER 8
#define A4 (F_CPU / (A4_FREQ * TIMER1_PRESCALER) - 1)

typedef enum {
    WAVE_NONE,
    WAVE_TRIANGLE,
    WAVE_SAW,
    WAVE_SINE,
    WAVE_NOISE
} waves;



typedef enum {
    FX_FLAG_DONE,
    FX_FLAG_LOOP,
    FX_FLAG_VALID,
    FX_FLAG_UNDERFLOW
} fx_flags_t;

typedef struct {
    fx_sound_t sound;
    volatile uint16_t sample;
    volatile uint16_t prev_sample;
    volatile uint8_t flags;
    uint16_t freq;
    uint16_t acc;
    uint16_t idx;
    voices_t voices[NVOICES];
} fx_state_t;

static volatile fx_state_t state = { 0 };

// TODO - make the player turn on and off the DAC between sounds.  For now it can just
// TODO - set the output to zero when nothing is playing.


// Sample width needs to match PWM setting.  Output will be shifted to
// fit in the selected PWM mode.
#define FX_SAMPLE_VALID 0x8000
#define FX_SAMPLE_WIDTH 8


static uint16_t fx_consume_next_sample(void);
static uint16_t fx_sound_static(void);
static uint16_t fx_sound_triangle(void);
static uint16_t fx_sound_triple_triangle(void);

void fx_init(void) {
    dac_set_sampler_cb(fx_consume_next_sample);
}

void fx_set_freq(uint16_t freq) {
    state.freq = freq;
    for (uint8_t i = 0; i < NVOICES; i++) {
        state.voices[i].freq = freq + (100 * i);
    }
}

void fx_play(fx_sound_t sound, bool loop) {
    state.sound = sound;
    state.idx = 0;
    state.flags = (loop) ? _BV(FX_FLAG_LOOP) : 0;
    dac_start();
}

void fx_stop(void) {
    dac_stop();
    if (state.flags & _BV(FX_FLAG_UNDERFLOW)) {
        printf("fx underflow!\n");
    }
    state.flags |= _BV(FX_FLAG_DONE);
}

bool fx_is_done(void) {
    return !!(state.flags & _BV(FX_FLAG_DONE));
}


/* Returns the next sample to be played for the currently playing
 * sound or 0 if nothing is playing.  If new sample isn't ready,
 * returns last sample to try to hide drop-out.  Called from interrupt.
 */
uint16_t fx_consume_next_sample(void) {

    if (state.flags & _BV(FX_FLAG_VALID)) {
        state.prev_sample = state.sample;
        state.flags &= ~_BV(FX_FLAG_VALID);
        return state.sample;
    } else {
        state.flags |= _BV(FX_FLAG_UNDERFLOW);
        return state.prev_sample;
    }
}

/* Calculates the value of the next sample, updates sample variable.
 * Called in the main loop.
 */
void fx_calc_next_sample(void) {
    if (state.flags & _BV(FX_FLAG_VALID)) {
        return;
    }

    switch(state.sound) {
    case FX_STATIC:
        state.sample = fx_sound_static();    
        break;
    case FX_TRIANGLE:
        state.sample = fx_sound_triangle();
        break;
    case FX_TRIPLE_TRIANGLE:
        state.sample = fx_sound_triple_triangle();
        break;
    default:
        return;
    }

    state.acc += state.freq;
    
    // for (uint8_t i = 0; i < NVOICES; i++) {
    //     state.voices[i].acc += state.voices[i].freq;
    //     state.voices[i].freq = (rand() % 100) << 10;
    // }

    state.flags |= _BV(FX_FLAG_VALID);
}

void fx_test(void) {
    uint32_t i = 0;
    fx_play(FX_STATIC, true);
    while(1) {
        if (i == 0xffffffff) {
            break;
        }
    }
    fx_stop();
}

static uint16_t fx_sound_static(void) {
    static uint16_t lfsr = 0xACE1;
    lfsr ^= lfsr >> 7;
    lfsr ^= lfsr << 9;
    lfsr ^= lfsr >> 13;
    return lfsr;
}

static uint16_t fx_sound_triangle(void) {
    uint16_t v;
    v = (state.acc & 0x8000) ? ~state.acc : state.acc;
    v = (v >> 4) & 0xFFF;
    return v;
}

static uint16_t fx_sound_triple_triangle(void) {
    uint16_t sample = 0;
    for (uint8_t i = 0; i < NVOICES; i++) {
        uint16_t acc = state.voices[i].acc;
        state.voices[i].acc = (acc & 0x8000) ? ~acc : acc;
        sample += state.voices[i].acc;
    }

    printf("%d ", sample);
    return sample;
}

// TODO - store this as a quarter-wave!
// TODO - either cut the space needed down to 64 bytes *OR* have a
// TODO - high resolution quarter that's still 256 bytes...  We have 32k
// TODO - of flash on a 328p (and I'm only using 11k so far), so maybe I don't care?
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
