#ifndef _CONFIG_H_
#define _CONFIG_H_


#define NEOPIXEL_CHAIN_PORT PORTD
#define NEOPIXEL_CHAIN_PIN PD7
#define NEOPIXEL_CHAIN_LENGTH 8

/* Neopixel timing fudgery */
#define T0H_ADJ -0.35
#define T1H_ADJ -0.32
#define TxL_ADJ -0.60

#define CONFIG_UART_BAUD 115200
// #define CONFIG_UART_TX_USE_IRQ
// #define CONFIG_UART_RX_USE_IRQ
// #define CONFIG_UART_TX_BUFFER_SZ 32
// #define CONFIG_UART_RX_BUFFER_SZ 32

#define CONFIG_DBG_HILO_ENABLE
#define CONFIG_DBG_HILO_DDR DDRD
#define CONFIG_DBG_HILO_PORT PORTD
#define CONFIG_DBG_HILO_BIT PD6
//#define CONFIG_DBG_UART_ENABLE
//#define CONFIG_DBG_UART_DDR  DDRD
//#define CONFIG_DBG_UART_PORT PORTD
//#define CONFIG_DBG_UART_BIT  PD6
//#define CONFIG_DBG_UART_BAUD 115200

#define CONFIG_SPI_PORT PORTB
#define CONFIG_SPI_DDR DDRB
#define CONFIG_SPI_MISO PB4
#define CONFIG_SPI_MOSI PB3
#define CONFIG_SPI_SS PB0
#define CONFIG_SPI_SCK PB5

/* DHT22 Temperature / Humidity Sensor */
#define CONFIG_DHT22_PORT PORTC
#define CONFIG_DHT22_PIN  PINC
#define CONFIG_DHT22_DDR  DDRC
#define CONFIG_DHT22_BIT  PC0

/* Buttons */
#define CONFIG_BUTTON_PORT       PORTC
#define CONFIG_BUTTON_DDR        DDRC
#define CONFIG_BUTTON_GREEN_PIN  PC1
#define CONFIG_BUTTON_YELLOW_PIN PC2
#define CONFIG_BUTTON_RED_PIN    PC3




#endif // _CONFIG_H_