#ifndef __SDCARD_H__
#define __SDCARD_H__

#include <assert.h>
#include <stdint.h>
#include <stddef.h>

typedef struct sdcard_s {
  

} sdcard_t;

void sdcard_init();
uint8_t sdcard_send_command(uint8_t idx, uint32_t arg);
uint8_t sdcard_read_resp();
int sdcard_read_block(size_t len, size_t addr, uint8_t *data);
void sdcard_crc_enable();
void sdcard_crc_disable();

#endif // __SDCARD_H__