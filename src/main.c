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

neopixels_t np_chain;

// struct app_state {
//     app_fsm_state_t fsm_state = STATE_IDLE;

// };



static void init(void)
{
    dbg_init();
    uart_init();
    spi_init();
    sdcard_init();
    // twi_master_init();

    // neopixels_init_chain(&np_chain, &NEOPIXEL_CHAIN_PORT,
    //                        NEOPIXEL_CHAIN_PIN, NEOPIXEL_CHAIN_LENGTH);
    sei();

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
        if (!btns) { continue };

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
        uint8_t digit_int, digit_frac;
        sample_unit_t unit;
        dht22_get_measurement(&digit_int, &digit_frac, &unit);
        print("digit_int: %0x\tdigit_frac: %0x", digit_int, digit_frac);
        data_unit = dht22_get_unit(dht22_data);
        
        display_set(digit_int, digit_frac, data_unit);
        /* Sleep 5 seconds at the end (locking out the keypad) before continuing. */


    }

}

buttons_t get_buttons() {
    // TODO - Figure out which pins on the test board can have buttons.
    // TODO - try to make them adjacent in a port.  They need to support
    // TODO - pin change interrupt, although we might not be using that yet.
    uint8_t buttons = BUTTON_PORT & BUTTON_MASK >> BUTTON_SHIFT;

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

/* Micocontroller firmware entry point */
int main(void) __attribute__((OS_main));
int main(void)
{
  

    init();
    dbg_hi();
    printf("alive!\n");
    dbg_lo();
    // anim_random_color_fader(&np_chain);


    while (1) {
        _delay_ms(1000);
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
