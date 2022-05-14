#include <stdio.h>

#ifndef _TWI_MASTER_H_
#define _TWI_MASTER_H_

#define TWI_FREQ 400000

inline void twi_master_init(void)
{
    /* No prescaling. */
    TWSR = 0;

    /* Set desired bus frequence based on clock. */
    TWBR = (F_CPU / (2 * TWI_FREQ)) - 8;

    /* Enable the TWI interface */
    TWCR = _BV(TWEN) | _BV(TWINT);
}

/* Block until TWINT is set and then clear it. */
inline void twi_block_on_twint(void)
{
    while(!(TWCR & _BV(TWINT)));
}

/* Send start condition on TWI */
inline void twi_start(void)
{
    TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN);
    twi_block_on_twint();
}

inline void twi_stop(void)
{
    TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN);
    while (TWCR & _BV(TWSTO));
}

inline void twi_ack_int(void)
{
    TWCR |= (_BV(TWINT) | _BV(TWEN) | _BV(TWINT));
}



void twi_master_send(uint8_t data);
void twi_master_write(uint8_t sla, uint16_t addr, uint8_t *data, uint8_t len);
void twi_master_read(uint8_t sla, uint16_t addr, uint8_t *data, uint8_t len);


#endif
