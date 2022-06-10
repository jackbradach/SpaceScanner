#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <avr/pgmspace.h>
#include <util/delay.h>

#include "anim.h"
#include "config.h"
#include "dbg.h"
#include "fx.h"


typedef struct {
    anim_t animation;
    uint32_t t_start_ms;
    uint8_t flags;
} anim_state_t;

void anim_init() {
    ftseg_init();
    fx_init();
}

void anim_is_done() {

}

void anim_start(anim_t which) {

}

void anim_update() {
    
}

