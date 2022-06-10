#ifndef __TIMER_H__
#define __TIMER_H__

#include <stdint.h>

void timer_init(void);
uint32_t get_ticks(void);
uint32_t get_ticks_ms(void);
#endif  // __TIMER_H__