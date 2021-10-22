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

    /* Set pin directions */
    SPI_DDR |= (SPI_MOSI | SPI_SCK | SPI_SS);
    SPI_DDR &= ~(SPI_MISO);

    /* Master mode, Fosc/2 speed */
    SPCR = (1 << SPE) | (1 << MSTR);
    SPSR |= (1 << SPI2X);
    // SPCR |= ((1 << SPE) | (1 << MSTR) );
}

uint8_t spi_xfer_byte(uint8_t tx) {
    SPDR = tx;
    while (!(SPSR & _BV(SPIF)));
    return SPDR;
}

uint16_t spi_xfer_word(uint16_t tx) {
    uint16_t word = 0;
    for (uint8_t i = 0; i < 2; i++) {
        SPDR = tx;
        while (!(SPSR & _BV(SPIF)));
        word |= ((uint16_t) SPDR << (8 * (1 - i)));
    }
    return word;
}

uint32_t spi_xfer_dword(uint32_t tx) {
    uint32_t dword = 0;
    for (uint8_t i = 0; i < 4; i++) {
        SPDR = tx;
        while (!(SPSR & _BV(SPIF)));
        dword |= ((uint32_t) SPDR << (8 * (3 - i)));
    }
    return dword;
}


