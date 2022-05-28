/* Jankity-ass TWI code; doesn't handle any errors yet or use interrupts! */

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#include "twi_master.h"
#define TWI_BUFSZ CONFIG_TWI_BUFSZ
// typedef enum {
//     TWI_FLAG_WRITE,
//     TWI_FLAG_READ

// } twi_flag_t;

typedef struct {
    volatile uint8_t sla;
    // uint8_t flags;
    uint8_t len;
    uint8_t *data;
} twi_state_t;

static twi_state_t twi_state = { 0 };

static inline void twi_ack_int(void) {
    TWCR == (_BV(TWINT) | _BV(TWEN) | _BV(TWIE));
}

void twi_master_init(void)
{
    /* No prescaling. */
    TWSR = 0;

    /* Set desired bus frequence based on clock. */
    TWBR = (F_CPU / (2 * TWI_FREQ)) - 8;

    /* Enable the TWI interface */
    TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWIE);

    twi_state.data = malloc(TWI_BUFSZ);
    twi_state.sla = 0;
}

void twi_send(uint8_t data) {
    TWDR = data;
    TWCR |= _BV(TWINT);
    twi_block_on_twint();
}

uint8_t twi_recv(bool ack) {
    if (ack) {
        TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWEA);
    } else {
        TWCR = _BV(TWEN) | _BV(TWINT);
    }
    twi_block_on_twint();
    return TWDR;
}

/* Master write, to sla/addr. */
// TODO - 2022/05/14 - jbradach - add error checking and make interrupt driven.
// TODO - should return a value to indicate number of bytes written or an error.
// void twi_master_mem_write(uint8_t sla, uint16_t addr, uint8_t *data, uint8_t len)
// {
//     twi_start();

//     /* [SLA][W][ADDR] */
//     twi_send(sla << 1);
//     twi_send(addr >> 8);
//     twi_send(addr & 0xFF);
//     for (uint8_t i = 0; i < len; i++) {
//         twi_send(data[i]);
//     }
//     twi_stop();
// }

/* Non-blocking write operation. */
void twi_master_write(uint8_t sla, uint8_t *data, uint8_t len) {
    /* Crappy multithread handling, basically if there's already
     * something in progress, block until done.  That's fine for
     * this project, since the main use of I2C is to write updates
     * to the 14-segment display.  For animations, I'm doing at
     * least 100ms as a frame delay.  It'd be weird if anything
     * ever got blocked here.
     */
    while (twi_state.sla || (TWCR & _BV(TWSTO)));

    /* Copy parameters to the state structure. */
    twi_state.sla = sla;
    twi_state.len = (len < TWI_BUFSZ) ? len : TWI_BUFSZ;
    memcpy(twi_state.data, data, twi_state.len);

    /* Kick it off; the ISR will do the rest. */
    TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN) | _BV(TWIE);
}

/* Master read, from sla/addr.  Returns number of bytes read (which should match len). */
// TODO - 2022/05/14 - jbradach - in addition to all the interrupt-driven mojo, this should do
// TODO - some amount of error checking.  Otherwise the return value is pointless since there
// TODO - can't ever be an error.
int twi_master_read(uint8_t sla, uint16_t addr, uint8_t *data, uint8_t len)
{
    twi_start();

    /* [SLA][W][ADDR] */
    twi_send(sla << 1);
    twi_send(addr >> 8);
    twi_send(addr & 0xFF);
    
    /* Repeated start (same as start!) */
    twi_start();

    /* [SLA][R][Dout] */
    twi_send(sla << 1| 1);

    /* Wait for data to be sent by the slave. */
    for (uint8_t i = 0; i < len; i++) {
        data[i] = twi_recv(!(i == (len-1)));
    }

    twi_stop();

    return len;
}



ISR(TWI_vect)
{
    static uint8_t idx;
    switch (TW_STATUS) {
    
    /* START condition has been transmitted */
    case TW_START:
        /* Load SLA+W and tell hardware to transmit */
        idx = 0;
        TWDR = twi_state.sla << 1;
        TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWIE);
        break;

    /* Repeated START condition has been transmitted */
    case TW_REP_START:
        /* Load SLA+R and tell hardware to transmit */
        TWDR = twi_state.sla << 1 | 1;
        TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWIE);
        break;

    /* TW_MT_SLA_ACK - Ack the slave and send the first byte of data.
     * If there is no payload (eg, this was an address probe), send stop.
     */
    case TW_MT_SLA_ACK:
        if (idx == twi_state.len) {
            twi_state.sla = 0;
            TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWSTO) | _BV(TWIE);
        } else {
            TWDR = twi_state.data[idx++];
            TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWIE);
        }
        break;
    
    /* TW_MT_SLA_NACK - The slave NACK'd, send a stop and release the bus. */
    case TW_MT_SLA_NACK:
        twi_state.sla = 0;
        TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWSTO) | _BV(TWIE);
        break;

    /* TW_MT_DATA_ACK - Send bytes until all have been transmitted, then
     * send a stop.
     */
    case TW_MT_DATA_ACK:
        if (idx < twi_state.len) {
            TWDR = twi_state.data[idx++];
            TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWIE);
        } else {
            TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWSTO);
            twi_state.sla = 0;
        }
        break;
    
    case TW_MT_DATA_NACK:
    case TW_MT_ARB_LOST:
    case TW_MR_SLA_ACK:
    case TW_MR_SLA_NACK:
    case TW_MR_DATA_ACK:
    case TW_MR_DATA_NACK:
    case TW_ST_SLA_ACK:
    case TW_ST_ARB_LOST_SLA_ACK:
    case TW_ST_DATA_ACK:
    case TW_ST_DATA_NACK:
    case TW_ST_LAST_DATA:
    case TW_SR_SLA_ACK:
    case TW_SR_ARB_LOST_SLA_ACK:
    case TW_SR_GCALL_ACK:
    case TW_SR_ARB_LOST_GCALL_ACK:
    case TW_SR_DATA_ACK:
    case TW_SR_DATA_NACK:
    case TW_SR_GCALL_DATA_ACK:
    case TW_SR_GCALL_DATA_NACK:
    case TW_SR_STOP:
    case TW_NO_INFO:
    case TW_BUS_ERROR:
        printf("TWI: ** WARK! **  TW_STATUS = %02x\n", TW_STATUS);
        twi_state.sla = 0;
        TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWSTO) | _BV(TWIE);
    }
}