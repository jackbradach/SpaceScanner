//FIXME: make each write not i2c based
// Holtek 16K33 driver
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <util/delay.h>

#include "twi_master.h"
#include "ht16k33.h"

#define HT16K33_I2C_ADDR_0 CONFIG_HT16K33_I2C_ADDR_0
#define HT16K33_I2C_ADDR_1 CONFIG_HT16K33_I2C_ADDR_1
#define HT16K33_I2C_ADDR_2 CONFIG_HT16K33_I2C_ADDR_2
#define HT16K33_I2C_ADDR_3 CONFIG_HT16K33_I2C_ADDR_3
#define HT16K33_MAX_DEVS 4
#define HT16K33_MAX_CH 4
#define HT16K33_I2C_TIMEOUT_MS 1000

#define WRITE_BIT I2C_MASTER_WRITE              /*!< I2C master write */
#define READ_BIT I2C_MASTER_READ                /*!< I2C master read */
#define ACK_CHECK_EN 0x1                        /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0                       /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0                             /*!< I2C ack value */
#define NACK_VAL 0x1                            /*!< I2C nack value */

typedef enum {
    HT16K33_CMD_SYSTEM_SETUP = 0x20,
    HT16K33_CMD_DISPLAY_SETUP = 0x80,
    HT16K33_CMD_DIMMING_SET = 0xE0,
} ht16k33_cmd_t;

static void ht16k33_write_cmd(ht16k33_device_handle_t *dev, uint8_t disp, uint8_t cmd);
static void write_display_setup(ht16k33_device_handle_t *dev, uint8_t disp);

// FIXME - add error checking to init!
void ht16k33_init(ht16k33_device_handle_t **dev) {
    ht16k33_device_handle_t *ht16k33;

    // FIXME - 2022/05/14 - jbradach - only allocate what we need based on ndevs and some
    // FIXME - abuse of the preprocessor to write the values.
    ht16k33 = malloc(sizeof(ht16k33_device_handle_t));
    ht16k33->i2c_devaddr[0] = HT16K33_I2C_ADDR_0;
    ht16k33->i2c_devaddr[1] = HT16K33_I2C_ADDR_1;
    ht16k33->i2c_devaddr[2] = HT16K33_I2C_ADDR_2;
    ht16k33->i2c_devaddr[3] = HT16K33_I2C_ADDR_3;

    for (int disp = 0; disp < CONFIG_HT16K33_NDEVS; disp++) {
        ht16k33_standby_exit(ht16k33, disp);
        ht16k33_set_brightness(ht16k33, disp, 16);
        ht16k33_set_blink_rate(ht16k33, disp, HT16K33_BLINK_OFF);
        ht16k33_clear(ht16k33, disp);
        ht16k33_update(ht16k33, disp);
        ht16k33_display_on(ht16k33, disp);
    }
    *dev = ht16k33;
}



void ht16k33_set_segments(ht16k33_device_handle_t *dev, uint8_t disp, uint8_t idx, uint16_t pattern) {
    dev->segments[disp][idx] |= pattern;
}

void ht16k33_clr_segments(ht16k33_device_handle_t *dev, uint8_t disp, uint8_t idx, uint16_t pattern) {
    dev->segments[disp][idx] &= pattern;
}

/* Adapted from the Arduino reference driver for the Sparkfun SPX-16425
 * "Qwiic Alphanumeric Display" quad 14-segment display.  The segments
 * are wired strangely, requiring some swizzling in the memory image.
 */
static void ht16k33_seg_to_ram_spx16425(ht16k33_device_handle_t *dev, uint8_t disp, uint8_t *buf) {
    uint8_t com, row;
    memset(buf, 0, HT16K33_RAM_SZ);
    for (uint8_t digit = 0; digit < HT16K33_DIGITS_PER_DEV; digit++) {
        uint16_t pattern = dev->segments[disp][digit];
        
        for (uint8_t segment = 0; segment < 14; segment++) {
            uint8_t addr;
            if (!(pattern & (1 << segment))) continue;
            com = segment;
            if (com > 6) com -= 7;
            if (7 == segment) com = 0;
            if (8 == segment) com = 1;
            row = digit;
            if (segment > 6) row += 4;
            addr = com * 2;
            if (row > 7) {
                addr++;
                row -= 8;
            }
           buf[addr] |= 1 << row;
        }
    }
}

/* Enable / disable the decimal point.  On this particular model, there's
 * only one decimal point, so I'm not bothering to handle all four yet.
 */
void ht16k33_enable_decimal_point(ht16k33_device_handle_t *dev, bool enable) {
    uint8_t data[2];
    data[0] = 0x3;
    data[1] = enable;
    twi_master_write(dev->i2c_devaddr[0], data, 2);
}

void ht16k33_update(ht16k33_device_handle_t *dev, uint8_t disp) {
    uint8_t data[17];
    data[0] = 0;
    ht16k33_seg_to_ram_spx16425(dev, disp, data + 1);
    twi_master_write(dev->i2c_devaddr[disp], data, 17);
}

// FIXME: needs to clear the segment buffer and the text?
void ht16k33_clear(ht16k33_device_handle_t *dev, uint8_t disp) {
    memset(dev->segments[disp], 0, sizeof(uint16_t) * HT16K33_DIGITS_PER_DEV);
}

void ht16k33_standby_enter(ht16k33_device_handle_t *dev, uint8_t disp) {
    ht16k33_write_cmd(dev, disp, HT16K33_CMD_SYSTEM_SETUP);
}

void ht16k33_standby_exit(ht16k33_device_handle_t *dev, uint8_t disp) {
    ht16k33_write_cmd(dev, disp, HT16K33_CMD_SYSTEM_SETUP | 1);
}

void ht16k33_set_brightness(ht16k33_device_handle_t *dev, uint8_t disp, int brightness) {
    // TODO: add assert on range 0-15.
    ht16k33_write_cmd(dev, disp, HT16K33_CMD_DIMMING_SET | brightness);
}

void ht16k33_set_blink_rate(ht16k33_device_handle_t *dev, uint8_t disp, int blink_rate) {
    dev->blink_rate = blink_rate;
    write_display_setup(dev, disp);
}

void ht16k33_display_on(ht16k33_device_handle_t *dev, uint8_t disp) {
    dev->display_on = 1;
    write_display_setup(dev, disp);
}

void ht16k33_display_off(ht16k33_device_handle_t *dev, uint8_t disp) {
    dev->display_on = 0;
    write_display_setup(dev, disp);
}

static void write_display_setup(ht16k33_device_handle_t *dev, uint8_t disp) {
    uint8_t display_setup;
    display_setup = HT16K33_CMD_DISPLAY_SETUP | (dev->blink_rate << 1) | dev->display_on;
    ht16k33_write_cmd(dev, disp, display_setup);
}

static void ht16k33_write_cmd(ht16k33_device_handle_t *dev, uint8_t disp, uint8_t cmd) {
    twi_master_write(dev->i2c_devaddr[disp], &cmd, 1);
}


