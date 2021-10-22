#ifndef __SDCARD_H__
#define __SDCARD_H__

#include <stdint.h>

typedef struct sdcard_s {
  

} sdcard_t;

void sdcard_init();
uint8_t sdcard_send_command(uint8_t idx, uint32_t arg);

#endif // __SDCARD_H__