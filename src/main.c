#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "config.h"
#include "dbg.h"
#include "uart.h"
#include "spi.h"
#include "neopixels.h"
#include "dac.h"
// #include "twi_master.h"
// #include "timer.h"

#include "pixels.h"
#if 0
#include "spi.h"
#include "pixels_anims.h"
#include "pixels_mods.h"
#include "neopixel_stick.h"
#include "neopixel_avr.h"
#include "led_gamma.h"

neopixel_stick np_stick;

pixels_anim anim_sine;


pixels_anim anim_startup;
#endif

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
buttons_t get_buttons();

neopixels_t np_chain;

// struct app_state {
//     app_fsm_state_t fsm_state = STATE_IDLE;

// };


// ** Timers section **
// TODO - move this section to timers.c!

void timers_init() {
    // Set up free-running timer for measuring microseconds.
    // 16mHz / prescaler = 

    // 16-bit running @ 20mHz, overflow at 20k -> 1khz.
    // timer_ms_tick = 0;
    // TCCR1A = 0;
    // TCCR1B = _BV(WGM12) | _BV(CS10);
    // OCR1A = 20000;
    // TIMSK1 |= _BV(OCIE1A);

    // TCCR0A = _BV(WGM01);
    // TCCR0B = _BV(CS01);
    // OCR0A = 250;

}

// ** DHT22 Section ** 
// TODO - move this section to dht22.c!

/* Structure for DHT22 measurements. */
#define DHT22_PORT CONFIG_DHT22_PORT
#define DHT22_DDR  CONFIG_DHT22_DDR
#define DHT22_PIN  CONFIG_DHT22_PIN
#define DHT22_BIT  CONFIG_DHT22_BIT

typedef struct {
    uint8_t rh_integral;
    uint8_t rh_decimal;
    uint8_t t_integral;
    uint8_t t_decimal;
} dht22_measurement_t;

bool dht22_read(dht22_measurement_t *meas);


bool dht22_init() {
    /* Set pin to input */
    DHT22_PORT |= _BV(DHT22_BIT);
    DHT22_DDR &= ~_BV(DHT22_BIT);

    /* Set up a timer with 1us resolution */
    TCCR0A = 0;
    TCCR0B = _BV(CS01);
    TIMSK0 = 0;
}

inline bool dht22_data() {
    return !!(DHT22_PIN & _BV(DHT22_BIT));
}

bool dht22_crc_check(uint32_t data, uint8_t crc) {
    uint8_t *bptr = (uint8_t *) &data;
    uint8_t sum = 0;
    for (uint8_t i = 0; i < 4; i++) {
        sum += bptr[i];
    }
    return (sum == crc);
}

bool dht22_read(dht22_measurement_t *meas) {
    uint32_t data = 0;
    uint8_t checksum = 0;
    uint8_t ticks_hi, ticks_lo;
    int16_t rh, t_c;

    /* Send start */
    DHT22_DDR |= _BV(DHT22_BIT);
    DHT22_PORT &= ~_BV(DHT22_BIT);
    _delay_ms(18);  // Note: datasheet said 500uS here?
    DHT22_PORT |= _BV(DHT22_BIT);
    DHT22_DDR &= ~_BV(DHT22_BIT);
    while(dht22_data());  // 80us low pulse from DHT
    while(!dht22_data());  // 80us high pulse from DHT ("get ready")
    while(dht22_data());

    /* Get data */
    for (int8_t bit = 31; bit >= 0; bit--) {
        uint8_t ticks;
        while(!dht22_data());
        TCNT0 = 0;
        while(dht22_data());
        ticks = TCNT0 >> 1;
        if (ticks > 50) {
            data |= 1ULL << bit;
            dbg_hi();
        } else {
            dbg_lo();
        }
    }

    /* Get checksum */
    for (int8_t bit = 7; bit >=0; bit--) {
        uint8_t ticks;
        while(!dht22_data());
        TCNT0 = 0;
        while(dht22_data());
        ticks = TCNT0 >> 1;
        if (ticks > 50) {
            checksum |= 1ULL << bit;
            dbg_hi();
        } else {
            dbg_lo();
        }
    }

    /* Verify the CRC was correct. */
    if (!dht22_crc_check(data, checksum)) {
        return false;
    }

    /* Unpack the measurement. */
    rh = data >> 16;
    t_c = data & 0xEFFF;
    meas->rh_integral = rh / 10;
    meas->rh_decimal = rh % 10;
    meas->t_integral = t_c / 10;
    meas->t_decimal = t_c % 10;
    if (data & 0x8000) {
        meas->t_integral *= -1;
    }

    return true;
}

void dht22_print(dht22_measurement_t *meas) {
    printf("T: %d.%dC  RH: %d.%d%%\n",
            meas->t_integral, meas->t_decimal,
            meas->rh_integral, meas->rh_decimal);
}

buttons_t get_buttons() {
    // TODO - Figure out which pins on the test board can have buttons.
    // TODO - try to make them adjacent in a port.  They need to support
    // TODO - pin change interrupt, although we might not be using that yet.
    // uint8_t buttons = BUTTON_PORT & BUTTON_MASK >> BUTTON_SHIFT;

    return BUTTONS_NONE;
}


# if 1
int move_color(uint8_t *color, uint8_t target, int rate)
{
    if (target > *color) {
        *color += rate;
    } else if (target < *color) {
        *color -= rate;
    } else {
        return 0;
    }
    return 1;
}
#else
int move_color(uint8_t *color, uint8_t target, int rate)
{
    int c;
    c = *color;

    if (target > (c - rate)) {
        c += rate;
    } else if (target < c) {
        c -= rate;
    } - USB 
    }
    return 1;
}
#endif

void anim_random_color_fader(neopixels_t *chain)
{
    uint8_t tgt_color[NEOPIXEL_CHAIN_LENGTH][3];
    int rate = 1;

    // 1. Random targets (per led)
    // 2. Move color towards target.
    // 3. If any colors are still moving, repeat.
    // 4. loop
    while (1) {
        uint8_t done;
        for (uint8_t i = 0; i < NEOPIXEL_CHAIN_LENGTH; i++) {
            for (uint8_t c = 0; c < PIXELS_CH_MAX; c++) {
                tgt_color[i][c] = rand();
            }
        }
        do {
            uint8_t i;
            done = 1;
            for (i = 0; i < NEOPIXEL_CHAIN_LENGTH; i++) {
                for (uint8_t c = 0; c < 3; c++) {
                    if (move_color((&(chain->color[i])[c]), tgt_color[i][c], rate)) {
                        done = 0;
                    }
                }
            }
            neopixels_refresh(&np_chain);
            _delay_ms(10.0);
        } while (!done);
    }
}


/* Call all the component init functions. */
static void init(void)
{
    dbg_init();
    uart_init();
    dht22_init();
    // spi_init();
    // sdcard_init();
    // twi_master_init();

    // neopixels_init_chain(&np_chain, &NEOPIXEL_CHAIN_PORT,
    //                        NEOPIXEL_CHAIN_PIN, NEOPIXEL_CHAIN_LENGTH);
    sei();
    // printf("** Alive!! **\n");
    
}

/* Micocontroller firmware entry point */
int main(void) __attribute__((OS_main));
int main(void)
{
  

    init();
    dbg_hi();
    printf("\n** Alive!! **\n");
    dbg_lo();
    // anim_random_color_fader(&np_chain);

    /* Main application loop! */
    while (1) {

        // Any event messages pending?  Dequeue and process.
        // TODO later - just start with polling!
        // switch (event_msg) {
        // case BUTTON_DOWN:

        // case BUTTON_UP:
        // case STARTUP:
        // default:
        // }
        // if (!buttons_pending) {

        // }

        // get_buttons is a simple debouncer that reads all three buttons
        // and returns an button_t.  It is responsible for handling
        // priority (which should go left-to-right across the device)
        // This function should block until the pin change interrupt fires.
        // Maybe go to sleep if we can to save battery?
        uint8_t btns = get_buttons();
        btns = 0x1;
        // find first set bit
        // We don't need this until the read is complete!  It's only for display.

        /* If no button is pressed, just go back up. Or sleep? */
        if (!btns) { continue; };

        /* This should just read one sample for now... */
        // TODO - decode dht2_read here based on button.
        // TODO - need to find 14-segment driver code from other project.
        // TODO - can just print to serial for now.
        // TODO - need a timestruct, since there could be more digits than can be displayed.
        // TODO - for now just display XX.YC
        // DHT22 protocol is timing-sensitive, this'll be fun...

        // TODO - make dht22_read return a bogus value for now, work on display next.
        
        // Do some masking fuckery on the DHT22 data to extract the needed unit.
        // Wait, 
        dht22_measurement_t meas;
        dht22_read(&meas);
        dht22_print(&meas);
        
        
        // display_set(digit_int, digit_frac, data_unit);
        /* Sleep 5 seconds at the end (locking out the keypad) before continuing. */
        _delay_ms(2000);
    }
   
//     neopixel_stick_off(&np_stick);
//     neopixel_refresh(&np_stick);
//     pixels_mods_clear_all(&np_stick);
// //    startup_anim();
// //
//     pixels_anim_init(&anim_startup, &pixels_anim_cb_flicker);
//     pixels_anim_set_oneshot(&anim_startup);
//     pixels_anim_enable_channels(&anim_startup, _BV(PIXELS_CH_BLUE) | _BV(PIXELS_CH_RED));
//     pixels_anim_set_range(&anim_startup, 0, 0x80);
//     pixels_anim_play(&anim_startup);

//     pixels_anim_init(&anim_sine, &pixels_anim_cb_offset_table);
//     pixels_anim_set_loop(&anim_sine);
//     pixels_anim_enable_channels(&anim_sine, _BV(PIXELS_CH_GREEN));
//     pixels_anim_set_table_id(&anim_sine, PIXELS_ANIM_SINEWAVE);
//     pixels_anim_play(&anim_sine);
    // while(1) {
    //     pixels_anim_apply_state(&np_stick, &anim_startup);

    //     pixels_anim_apply_state(&np_stick, &anim_sine);


    //     neopixel_refresh(&np_stick);

}

// void startup_anim(void)
// {
//     pixels_anim anim_sine;

//     neopixel_stick_off(&np_stick);
//     neopixel_refresh(&np_stick);
//     pixels_mods_clear_all(&np_stick);

//     //_delay_ms(1000);

//     /* Startup animation */
//     pixels_anim_init(&anim_startup, &pixels_anim_cb_flicker);
//     pixels_anim_set_oneshot(&anim_startup);
//     pixels_anim_enable_channels(&anim_startup, _BV(PIXELS_CH_BLUE) | _BV(PIXELS_CH_GREEN));
//     pixels_anim_set_range(&anim_startup, 0, 0x80);
//     pixels_anim_play(&anim_startup);

//     pixels_anim_init(&anim_sine, &pixels_anim_cb_offset_table);
//     pixels_anim_set_loop(&anim_sine);
//     pixels_anim_enable_channels(&anim_sine, _BV(PIXELS_CH_GREEN));
//     pixels_anim_set_table_id(&anim_sine, PIXELS_ANIM_SINEWAVE);
//     pixels_anim_play(&anim_sine);

// }
