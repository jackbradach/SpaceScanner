#include <stdint.h>

#include "sdcard.h"
#include "spi.h"


/* These are the two fixed bits for the command frame fields. */
#define SDCARD_INDEX_FIXED (1 << 6)
#define SDCARD_INDEX_MASK (0xCF)
#define SDCARD_CRC_FIXED (1 << 0)
#define SDCARD_CRC_MASK (0x7F)

/* The SDCard should respond in 0-8 bytes */
#define SDCARD_MAX_RESP_TIME 8

/* These are the SDCard SPI command indexes */
typedef enum {
    GO_IDLE_STATE,
    SEND_OP_COND,
    APP_SEND_OP_COND,
    SEND_IF_COND,
    SEND_CSD,
    SEND_CID,
    STOP_TRANSMISSION,
    SET_BLOCKLEN,
    READ_SINGLE_BLOCK,
    READ_MULTIPLE_BLOCK,
    SET_BLOCK_COUNT,
    SET_WR_BLOCK_ERASE_COUNT,
    WRITE_BLOCK,
    WRITE_MULTIPLE_BLOCK,
    APP_CMD,
    READ_OCR
} sdcard_cmd_t;

void sdcard_init() {
    sdcard_send_command(SEND_OP_COND, 0);
}

uint8_t sdcard_send_csd() {
    // return sdcard_send_command();
}


uint8_t sdcard_send_cid() {
    
}

uint8_t sdcard_send_command(uint8_t idx, uint32_t arg) {
    uint8_t crc;
    uint8_t resp;
    idx = SDCARD_INDEX_FIXED | (idx & SDCARD_INDEX_MASK);
    crc = SDCARD_CRC_FIXED;
    spi_slave_select();
    spi_xfer_byte(idx);
    spi_xfer_dword(arg);
    spi_xfer_byte(crc);

    for (int i = 0; i <= SDCARD_MAX_RESP_TIME; i++) {
        resp = spi_xfer_byte(0);
        printf("%i: resp: %02x\n", i, resp);
        if (resp != 0xff) {
            break;
        }
    }

    spi_slave_deselect();
    return resp;
}

int sdcard_set_blocklen(uint32_t len) {
    sdcard_send_command(SET_BLOCKLEN, len);
}

