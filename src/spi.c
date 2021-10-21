#include <stdbool.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>

#include "spi.h"

void spi_init() {
    /* Set initial values on pins before turning on the drivers. */
    SPI_PORT &= ~SPI_MOSI;
    SPI_PORT |= (SPI_MISO | SPI_SS);
    spi_slave_deselect();

    /* SCK and slave selects need to be output */
    SPI_DDR |= (SPI_MOSI | SPI_SCK | SPI_SS);
    SPI_DDR &= ~(SPI_MISO);
    SPI_SS_DDR |= SPI_SLAVES;

    /* Master mode, Fosc/4 speed */
    SPCR |= (_BV(SPE) | _BV(MSTR));
}

void spi_xfer_byte(uint8_t tx, uint8_t *rx) {
    SPCR |= (_BV(SPE) | _BV(MSTR));
    SPDR = tx;
    while (!(SPSR & _BV(SPIF)));
    if (rx)
        *rx = SPDR;
}


void spi_xfer_dword(uint32_t tx, uint32_t *rx) {
    uint32_t dword = 0;
    for (uint8_t i = 0; i < 4; i++) {
        uint8_t byte;
        spi_xfer_byte(0, &byte);
        dword |= ((uint32_t) byte << (8 * (3 - i)));
    }

    if (rx)
        *rx = dword;
}

void spi_xfer_word(uint16_t tx, uint16_t *rx) {
    uint16_t word = 0;
    for (uint8_t i = 0; i < 2; i++) {
        uint8_t byte;
        spi_xfer_byte(0, &byte);
        word |= ((uint16_t) byte << (8 * (1 - i)));
    }

    if (rx)
        *rx = word;
}
