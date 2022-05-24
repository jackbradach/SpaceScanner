#ifndef __FTSEG_H__
#define __FTSEG_H__

#include "ht16k33.h"

// FIXME - decide if this is going to be one device per screen or one device per
// FIXME - all of 'em.  I think the latter is probably easier to deal with.
// FIXME - Provide a DEFAULT() macro for config.
// FIXME - 
typedef struct ftseg_device_handle_s {
    ht16k33_device_handle_t *ht16k33;
    uint8_t disp_mask;
    char text[HT16K33_NDEVS][HT16K33_DIGITS_PER_DEV + 1];
} ftseg_device_handle_t;

// typedef enum {
//     ANIM_SEQ_VERT_SCROLL,
//     ANIM_SEQ_TYPE_HORZ_SCROLL,
//     ANIM_SEQ_BEAM_IN
//     // use the vert-scroll advanced across all the digits, each three-frame sequence
//     // ends with a digit 'beaming' in, moving from left to right.

// } anim_seq_t;

// typedef enum {
//     ANIM_FLAG_REVERSE = 0b001,    // Runs sequence in opposite direction
//     ANIM_FLAG_SEQUENTIAL = 0b010, // Changes characters one at a time
//     ANIM_FLAG_ADD_BLANK = 0b100,  // Adds a blank frame to the end
// } anim_flags_t;

// struct ftseg_anim {
//     anim_seq_t anim_seq;
//     uint32_t frame_delay;
//     anim_flags_t flags;
// };
// typedef struct ftseg_anim ftseg_anim_t;

typedef struct {
    char text[5];
    uint8_t disp;
} ftseg_evt_write_text_t;

typedef struct {
    uint16_t segments;
    uint8_t disp;
    uint8_t idx;
} ftseg_evt_write_segments_t;

/* Use when sending SET_BRIGHTNESS event. */
typedef struct {
    int8_t display_mask;
    uint8_t brightness;
} ftseg_evt_brightness_t;


int ftseg_init(ftseg_device_handle_t **dev);
void ftseg_destroy(ftseg_device_handle_t **dev);
void ftseg_puts(ftseg_device_handle_t *dev, const char *text);
void ftseg_write_text(ftseg_device_handle_t *dev, int disp, char *text);
void ftseg_test(ftseg_device_handle_t *dev, int disp);
uint16_t ftseg_anim_scan_start(uint8_t digit, uint8_t idx);
// ESP_EVENT_DECLARE_BASE(FTSEG_EVENTS);
// enum {
//     FTSEG_EVENT_WRITE_TEXT,
//     FTSEG_EVENT_WRITE_SEGMENTS,
//     FTSEG_EVENT_SET_BRIGHTNESS,

// };


#endif // __FTSEG_H__