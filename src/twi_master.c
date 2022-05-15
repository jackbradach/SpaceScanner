/* Jankity-ass TWI code; doesn't handle any errors yet or use interrupts! */

#include <stdbool.h>

#include <avr/io.h>
#include <util/twi.h>

#include "twi_master.h"


void twi_send(uint8_t data)
{
    TWDR = data;
    TWCR = _BV(TWEN) | _BV(TWINT);
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
void twi_master_write(uint8_t sla, uint16_t addr, uint8_t *data, uint8_t len)
{
    twi_start();

    /* [SLA][W][ADDR] */
    twi_send(sla << 1);
    twi_send(addr >> 8);
    twi_send(addr & 0xFF);
    for (uint8_t i = 0; i < len; i++) {
        twi_send(data[i]);
    }
    twi_stop();
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

