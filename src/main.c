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

typedef enum {
    RESET = 0,
    IDLE,
    READ_DHT22,
    FORMAT_TEXT,
    WRITE_FTSEG,
    WAIT_NOBUTTONS,
} fsm_t;

fsm_t fsm = RESET;
// struct app_state {
//     app_fsm_state_t fsm_state = STATE_IDLE;

// };

#define DELAY_LOOP 10
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

ISR(PCINT2_vect)
{
    buttons = (~BUTTONS_PIN & BUTTONS_MASK) >> BUTTONS_SHIFT;;
    printf("%2x\n", buttons);
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
    ftseg_init(&ftseg);
    buttons_init();
    // wdt_enable(WDTO_2S);
    sei();
}

void format_text(char *text, dht22_measurement_t *meas, uint8_t buttons);

fsm_t next();

/* Micocontroller firmware entry point */
int main(void) __attribute__((OS_main));
int main(void)
{
  
    printf("\n** Alive!! **\n");

    /* Main application loop! */
    while (1) {
        next();
                
        _delay_ms(DELAY_LOOP);
    }
   
}



/* Advance state machine. */
fsm_t next() {
    static uint8_t last_buttons;
    static char text[5];
    static dht22_measurement_t meas;
    
    fsm_t last = fsm;

    switch (fsm) {
    case RESET:
        init();
        fsm = IDLE;
        break;
    /* IDLE: no active buttons */
    case IDLE:
        if ((last_buttons = buttons)) {
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
        ftseg_write_text(ftseg, 0, text);
        fsm = WAIT_NOBUTTONS;
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
    printf("TEXT: %s\n", text);
}
