#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "ftseg.h"
#include "ftseg_data.c"

#define FTSEG_DISP_WIDTH 4

static const uint16_t ftseg_data_text[];


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

typedef enum {
    FTSEG_ANIM_NONE = 0,
    FTSEG_ANIM_SCAN_START,
    FTSEG_ANIM_SCAN_ACTIVE,
} ftseg_anim_t;

uint16_t ftseg_anim_scan_start(uint8_t digit, uint8_t idx);

// Returns pattern vector of pattern at index.
uint16_t ftseg_anim(int which, uint8_t digit, uint8_t idx) {
    uint16_t pattern;
    switch(which) {
    case FTSEG_ANIM_SCAN_START:
        pattern = ftseg_anim_scan_start(digit, idx);
        break;
    default:
        pattern = 0;
    }
    return pattern;
}

#define ANIM_SCAN_START_FRAMES 10
uint16_t ftseg_anim_scan_start(uint8_t digit, uint8_t idx) {
    uint16_t v;
    if (idx < ANIM_SCAN_START_FRAMES) {
        v = pgm_read_word(&ftseg_data_scan_start[idx]);
    } else {
        v = pgm_read_word(&ftseg_data_scan_start[9]);
    }
    printf("v: %04x\n", v);
    return v;
}

uint16_t ftseg_anim_scan_active(uint8_t digit, uint8_t idx) {
    // draw inverted starburst in middle of all digits.
    // Each digit should have random offset from others.
    // Outer ring "lock" is handled elsewhere.
    // if (idx < )
}

uint16_t ftseg_anim_scan_success(uint8_t digit, uint8_t idx) {
    // Flash [+] across digits?
}

uint16_t ftseg_anim_scan_fail(uint8_t digit, uint8_t idx) {
    // Flash [X] across digits that didn't lock.
    // USe [+] to show ones that did (no flash).
}


uint16_t ftseg_spinner(uint8_t idx) {
    return 1 << (idx % 15);
}


// esp_err_t ftseg_update(ftseg_device_handle_t *ftseg) {
//     ht16k33_device_handle_t *ht16k33 = ftseg->ht16k33;
//     const uint8_t sz = popcnt8(ftseg->disp_mask) * FTSEG_DISP_WIDTH;
//     char *text = ftseg->text;
//     uint8_t disp = 0;
//     uint8_t digit = 0;
//     uint8_t idx = 0;

//     // FIXME: this won't work with discontiguous displays.
//     // FIXME: add a virtual display lookup table and that
//     // FIXME: should fix it easy.
//     for (disp = 0; disp < HT16K33_NDEVS; disp++) {
//         ht16k33_clear(ht16k33, disp);
//     }
    
//     while (('\0' != text[idx]) && (idx < sz)) {
//         digit = idx % 4;
//         disp = idx / 4;
//         ht16k33_put_char(ht16k33, disp, digit, text[idx]);
//         idx++;
//     }

//     for (disp = 0; disp < HT16K33_NDEVS; disp++) {
//         ht16k33_update(ht16k33, disp);
//     }

//     return ESP_OK;
// }

// static void ftseg_event_handler(void* args, esp_event_base_t base, int32_t id, void *data) {
//     ftseg_device_handle_t *ftseg = args;
    
//     ESP_LOGI(TAG, "caught %s.%i -> %04x", base, id, *(uint16_t*) data);
//     switch (id) {
//         /* Writes a string to a ht16k33 display */
//         case FTSEG_EVENT_WRITE_TEXT: {
//             ftseg_evt_write_text_t *t = data;
//             ht16k33_clear(ftseg->ht16k33, t->disp);
//             for (uint8_t c = 0; c < strlen(t->text); c++) {
//                 ht16k33_set_segments(ftseg->ht16k33, t->disp, c, ftseg_data_text[(uint8_t) toupper(t->text[c])]);
//             }
//             ht16k33_update(ftseg->ht16k33, t->disp);
//             break;
//         }
//         /* Writes segments directly to a ht16k33 display */
//         case FTSEG_EVENT_WRITE_SEGMENTS: {
//             ftseg_evt_write_segments_t *s = data;
//             ht16k33_clr_segments(ftseg->ht16k33, s->disp, s->idx, 0xFFFF);
//             ht16k33_set_segments(ftseg->ht16k33, s->disp, s->idx, s->segments);
//             break;
//         }
//         /* Change brightness */
//         case FTSEG_EVENT_SET_BRIGHTNESS: {
//             ftseg_evt_brightness_t *b = data;
//             for (uint8_t disp = 0; disp < 4; disp++) {
//                 if (b->display_mask & (1 << disp)) {
//                     ht16k33_set_brightness(ftseg->ht16k33, disp, b->brightness);
//                 }
//             }
//             break;
//         }
//     }
// }

void ftseg_write_text(ftseg_device_handle_t *dev, int disp, char *text) {
    ht16k33_clear(dev->ht16k33, disp);
    for (uint8_t c = 0; c < strlen(text); c++) {
        ht16k33_set_segments(dev->ht16k33, disp, c, pgm_read_word(&ftseg_data_text[text[c]]));
    }
    ht16k33_update(dev->ht16k33, disp);
}

void ftseg_test(ftseg_device_handle_t *dev, int disp) {
    for (int i = 0; i < 16; i++) {
        printf("i = %d\n", i);
        // ht16k33_set_segments(dev->ht16k33, disp, 1, 1 << i);
        ht16k33_set_segments(dev->ht16k33, disp, 1, 0xffff);
        ht16k33_update(dev->ht16k33, disp);
        _delay_ms(2000);
    }
}

// FIXME - add error checking!
int ftseg_init(ftseg_device_handle_t **dev) {
    ftseg_device_handle_t *ftseg;

    ftseg = malloc(sizeof(ftseg_device_handle_t));
    // text_sz = (popcnt8(conf->disp_mask) * FTSEG_DISP_WIDTH) + 1;
    memset(ftseg->text, 0, sizeof(uint8_t) * HT16K33_NDEVS * (HT16K33_DIGITS_PER_DEV + 1));

    // ht16k33_conf.i2c_port = conf->i2c_port;
    ht16k33_init(&ftseg->ht16k33);
    // ftseg_update(ftseg);
    // ht16k33_set_brightness(&ftseg->ht16k33, 0, 16);
    
    *dev = ftseg;

    return 0;
}

void ftseg_destroy(ftseg_device_handle_t **dev) {
    if (NULL == dev) return;
    if (NULL == *dev) return;
    free(*dev);
    *dev = NULL;
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

