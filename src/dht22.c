#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>

#include "dht22.h"

static bool dht22_crc_check(uint32_t data, uint8_t crc) {
    uint8_t *bptr = (uint8_t *) &data;
    uint8_t sum = 0;
    for (uint8_t i = 0; i < 4; i++) {
        sum += bptr[i];
    }
    return (sum == crc);
}

static inline bool dht22_data() {
    return !!(DHT22_PIN & _BV(DHT22_BIT));
}

// TODO - make this share a timer with sound?
void dht22_init() {
    /* Set pin to input */
    DHT22_PORT |= _BV(DHT22_BIT);
    DHT22_DDR &= ~_BV(DHT22_BIT);

    /* Set up a timer with 1us resolution */
    TCCR0A = 0;
    TCCR0B = _BV(CS01);
    TIMSK0 = 0;
}

void dht22_print(dht22_measurement_t *meas) {
    printf("T: %d.%dC  RH: %d.%d%%\n",
            meas->t_integral, meas->t_decimal,
            meas->rh_integral, meas->rh_decimal);
}

/* Converts reading from celsius to fahrenheit, in place. */
void dht22_c_to_f(dht22_measurement_t *meas) {
    float c, f;
    c = (meas->t_integral * 10) + meas->t_decimal;
    f = ((9.0/5.0) * c) + 320.0;
    meas->t_integral = (int) f / 10;
    meas->t_decimal = (int) f % 10;
}

bool dht22_read(dht22_measurement_t *meas) {
    uint32_t data = 0;
    uint8_t checksum = 0;
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
