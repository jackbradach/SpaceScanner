#ifndef __DAC_H__
#define __DAC_H__

#include <stdint.h>

void dac_init();
void dac_set_sampler_cb(uint16_t (sampler_cb()));
void dac_set_value(uint16_t v);
void dac_start();
void dac_stop();
void dac_test();


#endif // __DAC_H__