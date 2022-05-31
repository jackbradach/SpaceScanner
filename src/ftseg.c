#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "timer.h"
#include "ftseg.h"
#include "ftseg_data.c"
#include "ht16k33.h"

#define FTSEG_DISP_WIDTH 4

static const uint16_t ftseg_data_text[];
ht16k33_device_handle_t *ht16k33;

#define FTSEG_ANIM_OUTLINE 0x3F
// XXX - 2022/05/23 - jbradach - what did I need popcnt for?
#if 0
/* 8-bit population count */
static uint8_t popcnt8(uint8_t v) {
    uint8_t cnt = 0;
    for (uint8_t i = 0; i < 8; i++) {
        if (v & 1) cnt++;
        v >>= 1;
        if (!v) break;
    }
    return cnt;
}
#endif

// Need to keep animation state.
// Need to track clean/dirty.  Every write causes a flicker.

#define FTSEG_TEXT_MAX 5

typedef struct {
    ftseg_anim_t anim;
    uint32_t t_start;
    uint32_t t_tick;
    uint16_t period_ms;
    volatile int8_t idx[HT16K33_DIGITS_PER_DEV];
    uint16_t pattern[HT16K33_DIGITS_PER_DEV];
    uint16_t text[HT16K33_DIGITS_PER_DEV];
    uint8_t brightness;
    uint8_t done;
} ftseg_anim_state_t;

#define IDX_UNSET -127
#define FTSEG_PATTERN_DOT _BV(14)

static ftseg_anim_state_t state = { 0 };

/* These get called by ftseg_anim_update based on the value of state.anim
 * and update the animation appropriately.
 */
void ftseg_anim_scan_start(void);
void ftseg_anim_scan_active(void);

/* Each digit has a done flag.  If all four are set, the overall
 * animation is done. */
bool ftseg_anim_is_done(void) {
    return (state.done == 0xF);
}

/* Call to set the current animation. */
void ftseg_anim_start(ftseg_anim_t which, uint16_t period_ms) {
    state.anim = which;
    for (int i = 0; i < HT16K33_DIGITS_PER_DEV; i++) {
        state.idx[i] = IDX_UNSET;
    }
    state.t_start = get_ticks_ms();
    state.t_tick = state.t_start;
    state.period_ms = period_ms;
    state.brightness = 0xF;
    memset(state.pattern, 0, 4 * sizeof(uint16_t));
    state.done = 0;
}

void ftseg_anim_update() {

    /* Only update every period_ms. */
    if (get_ticks_ms() - state.t_tick < state.period_ms) {
        return;
    }

    state.t_tick = get_ticks_ms();    
    switch(state.anim) {
    case FTSEG_ANIM_SCAN_START:
        ftseg_anim_scan_start();
        break;
    case FTSEG_ANIM_SCAN_ACTIVE:
        ftseg_anim_scan_active();
        break;

    case FTSEG_ANIM_SCAN_SUCCESS:
        break;
    default:
        return;
    }
    
    ht16k33_clear(ht16k33, 0);
    ht16k33_set_brightness(ht16k33, 0, state.brightness);
    for (int d = 0; d < HT16K33_DIGITS_PER_DEV; d++) {
        ht16k33_set_segments(ht16k33, 0, d, state.pattern[d]);
    }
    ht16k33_update(ht16k33, 0);

}

void ftseg_anim_scan_start(void) {
    if (state.idx[0] == IDX_UNSET) {
       state.brightness = 0xF;
       for (int d = 0; d < HT16K33_DIGITS_PER_DEV; d++) {
           state.idx[d] = 0;
       }
    }

    /* The offsets here look neat, kind of a galloping in from the left. */
    for (int d = 0; d < HT16K33_DIGITS_PER_DEV; d++) {
        uint8_t idx;

        // printf("d: %d  idx[d]: %d  done: %02x\n", d, state.idx[d], state.done);
        if (state.done & _BV(d)) {
            continue;
        }

        if (state.idx[d] - 2 * d > 0) {
            idx = state.idx[d] - 2 * d;
        } else {
            idx = 0;
        }

        if (!(idx < ANIM_SCAN_START_FRAMES)) {
            idx = ANIM_SCAN_START_FRAMES - 1;
            state.done |= _BV(d);
        }
        
        state.pattern[d] = pgm_read_word(&ftseg_data_scan_start[idx]);
        state.idx[d]++;
        // printf("---\n");
    }
}

// Scan active should reveal one digit per second, a box locking around each.  On the last one,
// it should drop the middle upright segments and start flashing like a box around the whole thing.
// Or should it box around each and then reveal them as it scan them? Maybe extend the box around
// the new "locked in" values.
// Fail animation should fade out with a "BRT BRT" or something
// Animate outline spinner for 30 seconds or so after a success.
// Change them to + (with a box extend) as they go.  Flash X on fail boxes.

#define ANIM_SCAN_ACTIVE_FRAMES (sizeof(ftseg_data_scan_active) / sizeof(uint16_t))
void ftseg_anim_scan_active(void) {
    if (state.idx[0] == IDX_UNSET) {
       state.brightness = 0xF;
       for (int d = 0; d < HT16K33_DIGITS_PER_DEV; d++) {
           state.idx[d] = 2 * d;
       }
    }

    /* The offsets here look neat, kind of a galloping in from the left. */
    for (int d = 0; d < 4; d++) {
        uint8_t idx;
        if (state.done & _BV(d)) {
            continue;
        }

        // printf("d: %d  idx[d]: %d  done: %02x\n", d, state.idx[d], state.done);
        // FIXME - add a random offset to the state struct for things like this.
        if (get_ticks_ms() - state.t_start > ((d * 1000) + 750)) {
            state.pattern[d] |= FTSEG_ANIM_OUTLINE;
        } else {
            state.pattern[d] = pgm_read_word(&ftseg_data_scan_active[state.idx[d]]);
        }
        state.idx[d] = ++state.idx[d] % ANIM_SCAN_ACTIVE_FRAMES;
    }
}

uint16_t ftseg_anim_scan_success(uint8_t digit, uint8_t idx) {
    // Flash [+] across digits?
}

uint16_t ftseg_anim_scan_fail(uint8_t digit, uint8_t idx) {
    // Flash [X] across digits that didn't lock.
    // USe [+] to show ones that did (no flash).
}

/* Converts a string into the patterns needed to show
 * the characters.  Periods are converted to the decimal
 * point on the character they follow.
 */
void ftseg_set_text(char *text) {
    uint8_t len = strlen(text);
    ht16k33_clear(ht16k33, 0);

    // Needs to handle decimal point
    for (int i, d = 0; i < len; i++) {
        /* If the next character is a period, set the bit
         * for the 'point' segment and skip a character.
         */
        state.text[d] = pgm_read_word(&ftseg_data_text[text[i]]);
        if ((i + 1 < len) && (text[i+1] == '.')) {
            state.text[d] |= FTSEG_PATTERN_DOT;
            i++;
        }
        d++;
    }
    for (int i = 0; i < 4; i++) {
        state.pattern[i] = state.text[i];
        ht16k33_set_segments(ht16k33, 0, i, state.pattern[i]);
        printf("%d: %04x\n", i, state.pattern[i]);
    }
    ht16k33_update(ht16k33, 0);

}

void ftseg_write_text(char *text) {
    ht16k33_clear(ht16k33, 0);
    for (uint8_t c = 0; c < strlen(text); c++) {
        ht16k33_set_segments(ht16k33, 0, c, pgm_read_word(&ftseg_data_text[text[c]]));
    }
    ht16k33_update(ht16k33, 0);
}

void ftseg_enable_decimal_point(bool enable) {
    ht16k33_enable_decimal_point(ht16k33, enable);
}


void ftseg_test() {
    for (int i = 0; i < 16; i++) {
        printf("i = %d\n", i);
        // ht16k33_set_segments(dev->ht16k33, disp, 1, 1 << i);
        ht16k33_set_segments(ht16k33, 0, 3, 0xffff);
        ht16k33_update(ht16k33, 0);
        
        _delay_ms(2000);
    }
}

// FIXME - add error checking!
void ftseg_init() {
    // text_sz = (popcnt8(conf->disp_mask) * FTSEG_DISP_WIDTH) + 1;
    // memset(ftseg->text, 0, sizeof(uint8_t) * HT16K33_NDEVS * (HT16K33_DIGITS_PER_DEV + 1));

    // ht16k33_conf.i2c_port = conf->i2c_port;
    ht16k33_init(&ht16k33);
    // ftseg_update(ftseg);
    // ht16k33_set_brightness(&ftseg->ht16k33, 0, 16);

    return 0;
}

// write to virtual display, can be a single or multiple.
// allow ganging them together?

// void ftseg_puts(ftseg_device_handle_t *dev, const char *text) {
//     uint8_t maxlen;
//     maxlen = (popcnt8(dev->disp_mask) * FTSEG_DISP_WIDTH);
//     strncpy(dev->text, text, maxlen);
//     ftseg_update(dev);    
// }

// void ftseg_put_char(ftseg_device_handle_t *ftseg, uint8_t disp, int idx, char v) {
//     ht16k33_clr_segments()
// }

