#ifndef __DAC_H__
#define __DAC_H__

#include <stdint.h>

void dac_init();
void dac_set_value(uint16_t v);
void dac_test();


#endif // __DAC_H__