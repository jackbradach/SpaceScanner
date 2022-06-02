#ifndef __HT16K33_H__
#define __HT16K33_H__

#include <stdint.h>

#include "config.h"

#define HT16K33_NDEVS CONFIG_HT16K33_NDEVS
typedef enum {
    HT16K33_BLINK_OFF = 0x0,
    HT16K33_BLINK_2HZ,
    HT16K33_BLINK_1HZ,
    HT16K33_BLINK_0P5HZ
} alpha_blink_rate_t;

#define HT16K33_RAM_SZ 16
#define HT16K33_DIGITS_PER_DEV 4

// struct ht16k33_config {
// };
// typedef struct ht16k33_config ht16k33_config_t;

typedef struct ht16k33_device_handle {
    uint8_t display_on;
    uint8_t blink_rate;
    uint16_t segments[HT16K33_NDEVS][HT16K33_DIGITS_PER_DEV];
    uint8_t i2c_devaddr[HT16K33_NDEVS];
} ht16k33_device_handle_t;


// TODO - add 'mask segment' function.
// void ht16k33_mask()

/* These functions touch the displays when called. */
void ht16k33_init(ht16k33_device_handle_t **dev);
void ht16k33_update(ht16k33_device_handle_t *dev, uint8_t disp);
void ht16k33_standby_enter(ht16k33_device_handle_t *dev, uint8_t disp);
void ht16k33_standby_exit(ht16k33_device_handle_t *dev, uint8_t disp);
void ht16k33_set_brightness(ht16k33_device_handle_t *dev, uint8_t disp, int brightness);
void ht16k33_set_blink_rate(ht16k33_device_handle_t *dev, uint8_t disp, int blink_rate);
void ht16k33_display_on(ht16k33_device_handle_t *dev, uint8_t disp);
void ht16k33_display_off(ht16k33_device_handle_t *dev, uint8_t disp);

/* These functions modify the segment buffer */
void ht16k33_clear(ht16k33_device_handle_t *dev, uint8_t disp); // FIXME - this seems to be broken?
void ht16k33_put_char(ht16k33_device_handle_t *dev, uint8_t disp, int os, char v);

void ht16k33_set_segments(ht16k33_device_handle_t *dev, uint8_t disp, uint8_t idx, uint16_t pattern);
void ht16k33_clr_segments(ht16k33_device_handle_t *dev, uint8_t disp, uint8_t idx, uint16_t pattern);
void ht16k33_enable_decimal_point(ht16k33_device_handle_t *dev, bool enable);

#endif // __HT16K33_H__