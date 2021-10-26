#include <stdbool.h>
#include <stdint.h>

#include <util/delay.h>

#include "crc7.h"
#include "sdcard.h"
#include "spi.h"


/* These are the two fixed bits for the command frame fields. */
#define SDCARD_INDEX_FIXED (1 << 6)
#define SDCARD_INDEX_MASK (0xCF)
#define SDCARD_CRC_FIXED (1 << 0)
#define SDCARD_CRC_MASK (0x7F)

/* The SDCard should respond in 0-8 bytes */
#define SDCARD_MAX_RESP_TIME 8

#define SDCARD_DATA_TOKEN 0xFE
#define SDCARD_DUMMY_XFER 0xFF
#define SDCARD_CMD0_CRC 0x95


/* These are the SDCard SPI command indexes */
typedef enum {
    GO_IDLE_STATE = 0,
    SEND_OP_COND = 1,
    // APP_SEND_OP_COND,
    SEND_IF_COND = 8,
    SEND_CSD = 9,
    SEND_CID = 10,
    STOP_TRANSMISSION = 12,
    SET_BLOCKLEN = 16,
    READ_SINGLE_BLOCK = 17,
    READ_MULTIPLE_BLOCK = 18,
    SET_BLOCK_COUNT = 23,
    // SET_WR_BLOCK_ERASE_COUNT,
    WRITE_BLOCK = 24,
    WRITE_MULTIPLE_BLOCK = 25,
    APP_CMD = 55,
    READ_OCR = 58
} sdcard_cmd_t;

static bool crc_calculation_enable;


/* Initialize SD Card in SPI mode
 * See doc/sdinit.png for flowchart.
 */
void sdcard_init() {
    int rc;
    uint8_t crc;

    sdcard_crc_enable();

    /* Delay at least 1ms after power-up */
    _delay_ms(1);
    
    /* Send at least 74 dummy clocks with CS and MOSI high. */
    spi_slave_deselect();
    for (int i = 0; i < 10; i++) {
        spi_xfer_byte(SDCARD_DUMMY_XFER);
    }
    
    /* */
    rc = sdcard_send_command(GO_IDLE_STATE, 0);
    printf("reset rc = %i\n", rc);
    rc = sdcard_send_command(SEND_IF_COND, 0x1AA);
    printf("send_if = 0x%02x\n", rc);
    
    // rc = sdcard_set_blocklen(512);
    // printf("set blocklen rc = %i\n", rc);
    // rc = sdcard_read_block(512, 0, data);
    // printf("rc = %i\n", rc);
    // for (int i = 0; i < 16; i++) {
    //     printf("%02x = %02x\n", i, data[i]);
    // }
}

/* Sends a command (with optional argument) to SDCard. */
uint8_t sdcard_send_command(uint8_t idx, uint32_t arg) {
    uint8_t crc;
    uint8_t resp;
    idx = SDCARD_INDEX_FIXED | (idx & SDCARD_INDEX_MASK);
    crc = 0;
    if (crc_calculation_enable) {
        crc = crc7_add(crc, idx);
        crc = crc7_add(crc, (arg & 0xff000000) >> 24);
        crc = crc7_add(crc, (arg & 0x00ff0000) >> 16);
        crc = crc7_add(crc, (arg & 0x0000ff00) >> 8);
        crc = crc7_add(crc, (arg & 0x000000ff));
        crc = SDCARD_CMD0_CRC;
    }
    spi_slave_select();
    spi_xfer_byte(idx);
    spi_xfer_dword(arg);
    spi_xfer_byte(crc);
    resp = sdcard_read_resp();
    spi_slave_deselect();
    return resp;
}

uint32_t sdcard_send_if_cond() {
    
}

void sdcard_crc_enable() {
    crc_calculation_enable = true;
}

void sdcard_crc_disable() {
    crc_calculation_enable = false;
}

int sdcard_set_blocklen(uint32_t len) {
    return sdcard_send_command(SET_BLOCKLEN, len);
}

// TODO - check R1 response flags!
// Also need to handle R1b response
// Reads respond with an R1, a data block, and end with a CRC.

uint8_t sdcard_read_resp() {
    uint8_t resp;
    for (int i = 0; i <= SDCARD_MAX_RESP_TIME; i++) {
        resp = spi_xfer_byte(0);
        if (resp != 0xff) {
            break;
        }
    }
    return resp;
}

/* Read a block of size len from address.
 * [Data Token][ Data Block ][  CRC  ]
 *    1 byte    1-2048 bytes  2 bytes
 * 
 * Data token for most reads is 0xFE
 */
int sdcard_read_block(size_t len, size_t addr, uint8_t *data) {
    assert(data != NULL);
    uint8_t idx;
    uint8_t crc;
    uint8_t resp;
    idx = SDCARD_INDEX_FIXED | READ_SINGLE_BLOCK;
    crc = SDCARD_CRC_FIXED;
    spi_slave_select();
    spi_xfer_byte(idx);
    spi_xfer_dword(addr);
    spi_xfer_byte(crc);
    for (int i = 0; i <= SDCARD_MAX_RESP_TIME; i++) {
        resp = spi_xfer_byte(0);
        printf("resp=%i\n", resp);
        if (resp != SDCARD_DATA_TOKEN) {
            break;
        }
    }
    for (size_t b = 0; b < len; b++) {
        data[b] = spi_xfer_byte(0);
    }
    spi_slave_deselect();
    return resp;

}