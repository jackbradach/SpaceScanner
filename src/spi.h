#ifndef _SPI_H_
#define _SPI_H_

#include <stdbool.h>
#include <stdint.h>
#include <avr/io.h>
#include "config.h"

#define SPI_PORT CONFIG_SPI_PORTB
#define SPI_DDR CONFIG_SPI_DDR
#define SPI_SS _BV(PB4)
#define SPI_MOSI _BV(PB5)
#define SPI_MISO _BV(PB6)
#define SPI_SCK _BV(PB7)

// #define SPI_SS_DDR DDRA
// #define SPI_SS_PORT PORTA
// #define SPI_SS_TCAMP3 _BV(PA7)
// #define SPI_SS_TCAMP2 _BV(PA6)
// #define SPI_SS_TCAMP1 _BV(PA5)
// #define SPI_SS_TCAMP0 _BV(PA4)
// #define SPI_SLAVES (SPI_SS_TCAMP3 | SPI_SS_TCAMP2 | SPI_SS_TCAMP1 | SPI_SS_TCAMP0)

inline void spi_slave_deselect(void)
{
    // SPI_SS_PORT |= SPI_SLAVES;
}

void spi_init(void);
void spi_xfer_byte(uint8_t tx, uint8_t *rx);
void spi_xfer_dword(uint32_t tx, uint32_t *rx);
void spi_xfer_word(uint16_t tx, uint16_t *rx);

#endif
