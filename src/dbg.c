#include <stdio.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#include "config.h"
#include "dbg.h"

#if defined(CONFIG_DBG_UART_ENABLE)
#define US_PER_BIT (1000000UL / CONFIG_DBG_UART_BAUD)
static int dbg_putchar(char c, FILE *f);
static FILE dbgout = {0};
#endif

/* Set up DBG I/O and redirect stdout. */
void dbg_init()
{
#if defined(CONFIG_DBG_UART_ENABLE)
    /* Set pin output and drive mark (idle). */
    CONFIG_DBG_UART_PORT |= _BV(CONFIG_DBG_UART_BIT);
    CONFIG_DBG_UART_DDR |= _BV(CONFIG_DBG_UART_BIT);

    /* Map stdout to the debug port. */
    fdev_setup_stream (&dbgout, dbg_putchar, NULL, _FDEV_SETUP_WRITE);
    stdout = &dbgout;
#endif

#if defined(CONFIG_DBG_HILO_ENABLE)
    /* Enable the "HI / LO" debug line. */
    CONFIG_DBG_HILO_PORT &= ~_BV(CONFIG_DBG_HILO_BIT);
    CONFIG_DBG_HILO_DDR |= _BV(CONFIG_DBG_HILO_BIT);
#endif
}

#if defined(CONFIG_DBG_UART_ENABLE)
/* Debug putchar function, suitable for handing to fdev.
 * Interrupts probably ought to be disabled while running.
 */
static int dbg_putchar(char c, FILE *f)
{
    uint16_t symbol = (1 << 9U) | (c << 1U);

    cli();
    for (int i = 0; i < 10; i++) {
        if (symbol & 1) {
            DBG_PORT |= _BV(DBG_BIT);
        } else {
            DBG_PORT &= ~_BV(DBG_BIT);
        }
        symbol >>= 1;
        _delay_us(US_PER_BIT);
    }
    sei();
    return c;
}
#endif