#ifndef _SPI_H_
#define _SPI_H_

#include <stdbool.h>
#include <stdint.h>
#include <avr/io.h>
#include "config.h"

#define SPI_PORT CONFIG_SPI_PORT
#define SPI_DDR CONFIG_SPI_DDR
#define SPI_SS (1 << CONFIG_SPI_SS)
#define SPI_MOSI (1 << CONFIG_SPI_MOSI)
#define SPI_MISO (1 << CONFIG_SPI_MISO)
#define SPI_SCK (1 << CONFIG_SPI_SCK)

inline void spi_slave_select(void) {
    SPI_PORT &= ~SPI_SS;
}

inline void spi_slave_deselect(void) {
    SPI_PORT |= SPI_SS;
}


void spi_init(void);
uint8_t spi_xfer_byte(uint8_t tx);
uint16_t spi_xfer_word(uint16_t tx);
uint32_t spi_xfer_dword(uint32_t tx);

#endif
