#ifndef __DHT22_H__
#define __DHT22_H__

#include <stdbool.h>
#include <stdint.h>
#include "config.h"

#define DHT22_PORT CONFIG_DHT22_PORT
#define DHT22_DDR  CONFIG_DHT22_DDR
#define DHT22_PIN  CONFIG_DHT22_PIN
#define DHT22_BIT  CONFIG_DHT22_BIT

typedef struct {
    uint8_t rh_integral;
    uint8_t rh_decimal;
    uint8_t t_integral;
    uint8_t t_decimal;
} dht22_measurement_t;


void dht22_print(dht22_measurement_t *meas);
bool dht22_read(dht22_measurement_t *meas);
void dht22_init();

#endif  // __DHT22_H__