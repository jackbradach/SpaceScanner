#include <stdio.h>
#include <avr/interrupt.h>
#include <avr/io.h>

#include "config.h"
#if defined(CONFIG_UART_TX_USE_IRQ) || defined(CONFIG_UART_RX_USE_IRQ)
#include "rings.h"
#endif

// TODO - 2019/01/05 - interrupt support!

/* This works with the setbaud magic from libc. */
#define BAUD CONFIG_UART_BAUD

static int uart_putchar(char c, FILE *f);
static int uart_getchar(FILE *f);
static FILE uart_io = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);

#if defined(CONFIG_UART_TX_USE_IRQ)
ring_t *tx_ring;
#endif

#if defined(CONFIG_UART_RX_USE_IRQ)
ring_t *rx_ring;
#endif



void uart_init(void) {
    #include <util/setbaud.h>
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;
    #undef BAUD
    #if USE_2X
        UCSR0A = _BV(U2X0);
    #endif
    
    /* Enable TX/RX w/ 8-N-1 */
    UCSR0B |= _BV(TXEN0);
        #if defined(CONFIG_UART_TX_USE_IRQ)
        //    | _BV(TXCIE0)
        #endif
        #if defined(CONFIG_UART_RX_USE_IRQ)
        //    | _BV(RXCIE0)
        #endif
        //    | _BV(RXEN0);
    UCSR0C |= (_BV(UCSZ01) | _BV(UCSZ00));

    /* Redirect stdio to the uart */
    stdin = stdout = &uart_io;
#if defined(CONFIG_UART_TX_USE_IRQ)
    ring_init(&tx_ring, CONFIG_UART_TX_BUFFER_SZ);
#endif
#if defined(CONFIG_UART_RX_USE_IRQ)
    ring_init(&rx_ring, CONFIG_UART_TX_BUFFER_SZ);
#endif
}

/* Putchar needed for redirecting stdio. */
static int uart_putchar(char c, FILE *f) {
    /* Add in a carriage return because god forbid we
     * stop supporting CP/M style line endings.
     */
    if ('\n' == c)
        uart_putchar('\r', f);

#if defined(CONFIG_UART_TX_USE_IRQ)
    /* If the buffer's full, this will stall. */
    ring_put(tx_ring, c);
    UCSR0B |= _BV(UDRIE0);
#else
    /* Wait for uart data register empty flag */
    while (!(UCSR0A & _BV(UDRE0)));
    UDR0 = c;
#endif
    return (int) c;
}

/* Getchar needed for redirecting stdio. */
static int uart_getchar(FILE *f) {
#if defined(CONFIG_UART_TX_USE_IRQ)
    uint8_t c;
    /* This will block until data is available. */
    ring_get(tx_ring, &c);
    return (int) c;
#else
    /* Wait for data received flag. */
    while (!(UCSR0A & _BV(RXC0)));
    return (int) UDR0;
#endif
}

/* Note: Since there's only a single byte buffer, the overhead
 * of the ISR exceeds that of just sitting and spinning on UDRE
 * at around 38.4k!
 */
#ifdef CONFIG_UART_TX_USE_IRQ
ISR(USART_UDRE_vect) {
    /* Disable data reg empty interrupt */
    UCSR0B &= ~_BV(UDRIE0);
    if (!ring_isempty(tx_ring)) {
        uint8_t c;
        ring_get(tx_ring, &c);
        UDR0 = c;
        UCSR0B |= _BV(UDRIE0);
    }
}
#endif

