#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "config.h"
#include "dac.h"
#include "dbg.h"
#include "dht22.h"
#include "ftseg.h"
#include "neopixels.h"
#include "spi.h"
#include "uart.h"
#include "twi_master.h"
// #include "timer.h"
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

ftseg_device_handle_t *ftseg;

// struct app_state {
//     app_fsm_state_t fsm_state = STATE_IDLE;

// };

#define DELAY_LOOP 1000
#define BUTTONS_PORT PORTD
#define BUTTONS_PIN PIND
#define BUTTONS_DDR DDRD
#define BUTTONS_MASK 0x1C
#define BUTTONS_SHIFT 2

// PCINT18 -> PD2 (Green)
// PCINT19 -> PD3 (Yellow)
// PCINT20 -> PD4 (Red)


buttons_t buttons_get() {
    // TODO - Figure out which pins on the test board can have buttons.
    // TODO - try to make them adjacent in a port.  They need to support
    // TODO - pin change interrupt, although we might not be using that yet.
    // uint8_t buttons = BUTTONS_PORT & BUTTONS_MASK >> BUTTONS_SHIFT;

    return BUTTONS_NONE;
}

void buttons_init() {
    BUTTONS_DDR &= ~(_BV(PD4) | _BV(PD3) | _BV(PD2));
    BUTTONS_PORT |= _BV(PD4) | _BV(PD3) | _BV(PD2);

    PCMSK2 |= _BV(PCINT20) | _BV(PCINT19) | _BV(PCINT18);
    PCICR |= _BV(PCIE2);
}

ISR(PCINT2_vect)
{
    // Your code here
    printf("%0x\n", PIND);
}

/* Call all the component init functions. */
static void init(void)
{
    dbg_init();
    uart_init();
    dht22_init();
    // spi_init();
    // sdcard_init();
    twi_master_init();

    buttons_init();
    // wdt_enable(WDTO_2S);
    sei();
}

/* Micocontroller firmware entry point */
int main(void) __attribute__((OS_main));
int main(void)
{
  
    init();

    printf("\n** Alive!! **\n");
    ftseg_init(&ftseg);

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

        // buttons_get is a simple debouncer that reads all three buttons
        // and returns an button_t.  It is responsible for handling
        // priority (which should go left-to-right across the device)
        // This function should block until the pin change interrupt fires.
        // Maybe go to sleep if we can to save battery?
        uint8_t btns = buttons_get();
        btns = 0x1;
        // find first set bit
        // We don't need this until the read is complete!  It's only for display.

        /* If no button is pressed, just go back up. Or sleep? */
        if (!btns) { continue; };

        /* Take an environmental reading. */
        dht22_measurement_t meas;
        dht22_read(&meas);
        dht22_print(&meas);
        
        // display_set(digit_int, digit_frac, data_unit);
        // uint8_t testo[] = {0xAA, 0xBB, 0xCC, 0xDD};
        // uint8_t i2c_rd_data[4];
        // #define HT16K33_I2C_ADDR 0x71
        // twi_master_write(HT16K33_I2C_ADDR, 0x1234, testo, 4);
        // twi_master_read(HT16K33_I2C_ADDR, 0x1234, i2c_rd_data, 4);

        
        // ht16k33_display_off(ht16k33, 0);
        char text[5];
        sprintf(text, "RH%02d", meas.rh_integral);
        ftseg_write_text(ftseg, 0, text);
        _delay_ms(DELAY_LOOP);
        sprintf(text, "C%02d%d", meas.t_integral, meas.t_decimal);
        ftseg_write_text(ftseg, 0, text);
        _delay_ms(DELAY_LOOP);

        dht22_c_to_f(&meas);
        sprintf(text, "F%02d%d", meas.t_integral, meas.t_decimal);
        ftseg_write_text(ftseg, 0, text);

        // ftseg_test(ftseg,0);
        /* Sleep 5 seconds at the end (locking out the keypad) before continuing. */
        _delay_ms(DELAY_LOOP);
    }
   
}

