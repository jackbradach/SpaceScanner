#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>

#include "config.h"
#include "dac.h"
#include "dbg.h"
#include "dht22.h"
#include "ftseg.h"
#include "ht16k33.h"
#include "neopixels.h"
#include "spi.h"
#include "uart.h"
#include "twi_master.h"
#include "timer.h"
#include <avr/wdt.h> 

/* Application state; extend as needed! */
// typedef enum {
//     STATE_IDLE,
//     STATE_SAMPLING,
//     STATE_AVERAGING,
//     STATE_SHOW
// } app_fsm_state_t;

typedef enum {
    BUTTONS_NONE = 0x0,
    BUTTONS_GREEN = 0x1,
    BUTTONS_YELLOW = 0x2,
    BUTTONS_RED = 0x4
} buttons_t;
buttons_t buttons_get();

uint8_t buttons;

extern ht16k33_device_handle_t *ht16k33;

typedef enum {
    RESET = 0,
    IDLE,
    FADE_DOWN_FTSEG,
    SCANNING_START,
    SCANNING_ACTIVE,
    SCANNING_SUCCESS,
    SCANNING_FAIL,
    SCANNING,
    READ_DHT22,
    FORMAT_TEXT,
    WRITE_FTSEG,
    FADE_UP_FTSEG,
    WAIT_NOBUTTONS,
} fsm_t;

fsm_t fsm = RESET;
// struct app_state {
//     app_fsm_state_t fsm_state = STATE_IDLE;

// };

#define FTSEG_FADE_PERIOD_MS 10UL
#define FTSEG_SPINNER_PERIOD_MS 50UL
#define BUTTONS_PORT PORTD
#define BUTTONS_PIN PIND
#define BUTTONS_DDR DDRD
#define BUTTONS_MASK 0x1C
#define BUTTONS_SHIFT 2

// PCINT18 -> PD2 (Green)
// PCINT19 -> PD3 (Yellow)
// PCINT20 -> PD4 (Red)


void buttons_init() {
    BUTTONS_DDR &= ~(_BV(PD4) | _BV(PD3) | _BV(PD2));
    BUTTONS_PORT |= _BV(PD4) | _BV(PD3) | _BV(PD2);

    PCMSK2 |= _BV(PCINT20) | _BV(PCINT19) | _BV(PCINT18);
    PCICR |= _BV(PCIE2);
}

ISR(PCINT2_vect) {
    buttons = (~BUTTONS_PIN & BUTTONS_MASK) >> BUTTONS_SHIFT;;
}

// bool prng_init = false;

void prng_init() {

    uint16_t seed = 0;
    DDRC = 0;
    ADMUX = _BV(REFS1) | _BV(REFS0) | _BV(MUX3);
    ADCSRA = _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);

    for (int i = 0; i < 16; i++) {
        ADCSRA |= _BV(ADSC);
        while(ADCSRA & _BV(ADSC));
        seed |= (ADC & 0x1) << i;
        _delay_ms(1);
    }
    srand(seed);
    ADCSRA = 0;
}

/* Call all the component init functions. */
static void init(void)
{
    dbg_init();
    timer_init();
    uart_init();
    prng_init();
    // dac_init();
    // spi_init();d
    // sdcard_init();
    twi_master_init();
    buttons_init();

   

    /* Inits past this point require interrupts. */
    sei();

    // do ADC read


    ftseg_init();
    dht22_init();
 

}


void format_text(char *text, dht22_measurement_t *meas, uint8_t buttons);

fsm_t next();

/* Micocontroller firmware entry point */
int main(void) __attribute__((OS_main));
int main(void)
{
    init();
    
    printf("\n** Alive!! **\n");
    // while (1) {
    //     /* Start scan*/
    //     ftseg_anim_start(FTSEG_ANIM_SCAN_START, 60);
    //     while (!ftseg_anim_is_done()) {
    //         ftseg_anim_update();
    //     }
    //     /* Active scan */
    //     ftseg_anim_start(FTSEG_ANIM_SCAN_ACTIVE, 60);
    //     while (!ftseg_anim_is_done()) {
    //         ftseg_anim_update();
    //     }
    //     _delay_ms(2000);
    // }


    // ht16k33_set_brightness(ht16k33, 0, 16);
    // for (int i = 0; i < 20; i++) {
    //     ht16k33_clear(ht16k33, 0);
    //     for (int d = 0; d < 4; d++) {
    //         uint16_t pat;
    //         uint8_t idx;
    //         if (i - 2 * d > 0) {
    //             idx = i - 2 * d;
    //         } else {
    //             idx = 0;
    //         }
    //         pat = ftseg_anim(FTSEG_ANIM_SCAN_START, d, i - 2*d);
    //         ht16k33_set_segments(ht16k33, 0, d, pat);
    //         ht16k33_update(ht16k33, 0);
    //     }
    //     _delay_ms(60);
    // }

    // for (uint8_t idx = 0; idx < 100 ; idx++) {
    //     ht16k33_clear(ht16k33, 0);
    //     for (int d = 0; d < 4; d++) {
    //         uint16_t pat;
    //         pat = ftseg_anim(FTSEG_ANIM_SCAN_ACTIVE, d, idx + 2 * d);
    //         ht16k33_set_segments(ht16k33, 0, d, pat);
    //         ht16k33_update(ht16k33, 0);
    //     }
    //     _delay_ms(60);
    // }
    // }
    // 0x3FC0

    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();

    /* Main application loop! */
    while (1) {
        next();
        twi_block_until_done();
        uart_block_until_done();
        sleep_cpu();
    }
}

/* Advance state machine. */
fsm_t next() {
    static uint8_t last_buttons;
    static char text[5];
    static dht22_measurement_t meas;
    static int8_t ftseg_brightness;
    static uint32_t t_start;
    static uint8_t idx;

    fsm_t last = fsm;

    switch (fsm) {
    case RESET:
        t_start = 0;
        ftseg_brightness = 0;
        fsm = IDLE;
        break;
    /* IDLE: no active buttons */
    case IDLE:
        if ((last_buttons = buttons)) {
            t_start = get_ticks_ms();
            fsm = FADE_DOWN_FTSEG;
        }
        break;

    /* Fade down any existing measurement. */
    case FADE_DOWN_FTSEG:
        if (ftseg_brightness > 0) {
            if (get_ticks_ms() - t_start > FTSEG_FADE_PERIOD_MS) {
                ftseg_brightness--;
                ht16k33_set_brightness(ht16k33, 0, ftseg_brightness);
                t_start = get_ticks_ms();
            }
        } else {
            // ftseg_write_text(ftseg, 0, "");
            ht16k33_clear(ht16k33, 0);
            ht16k33_update(ht16k33, 0);
            ftseg_anim_start(FTSEG_ANIM_SCAN_START, 60);
            fsm = SCANNING_START;
        }
        break;

    case SCANNING_START:
        if (ftseg_anim_is_done()) {
            ftseg_anim_start(FTSEG_ANIM_SCAN_ACTIVE, 60);
            fsm = SCANNING_ACTIVE;
        } else {
            ftseg_anim_update();
        }
        break;

    case SCANNING_ACTIVE:
        ftseg_anim_update();
        if (!buttons) {
            fsm = READ_DHT22;
        }
        break;

    case READ_DHT22:
        dht22_read(&meas);
        fsm = FORMAT_TEXT;
        break;
    case FORMAT_TEXT:
        format_text(text, &meas, last_buttons);
        fsm = WRITE_FTSEG;
        break;

    case WRITE_FTSEG:
        ftseg_write_text(text);
        fsm = FADE_UP_FTSEG;
        break;

    /* Fade up with the new measurement. */
    case FADE_UP_FTSEG:
        if (ftseg_brightness <= 16) {
            if ((get_ticks_ms() - t_start) > FTSEG_FADE_PERIOD_MS) {
                ftseg_brightness++;
                ht16k33_set_brightness(ht16k33, 0, ftseg_brightness);
                t_start = get_ticks_ms();
            }
        } else {
            fsm = WAIT_NOBUTTONS;
        }
        break;

    case WAIT_NOBUTTONS:
        if (!buttons) {
            fsm = IDLE;
        }
        break;
    default:
        fsm = RESET;
    }
    if (last != fsm) {
        printf("FSM: %d->%d\n", last, fsm);
    }
    return fsm;
}

void format_text(char *text, dht22_measurement_t *meas, uint8_t buttons) {

    /* If only one button was pushed, write the corresponding string
     * to the buffer.
     */
    switch (buttons) {
    case BUTTONS_GREEN:
        sprintf(text, "C%02d%d", meas->t_integral, meas->t_decimal);
        break;
    case BUTTONS_YELLOW:
        sprintf(text, "RH%02d", meas->rh_integral);
        break;
    case BUTTONS_RED:
        dht22_c_to_f(meas);
        sprintf(text, "F%02d%d", meas->t_integral, meas->t_decimal);
        break;
    default:
        memset(text, 0, 5);
    }
}
