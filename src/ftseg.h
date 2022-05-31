#ifndef __FTSEG_H__
#define __FTSEG_H__

#include <stdbool.h>
#include <stdint.h>

// FIXME - decide if this is going to be one device per screen or one device per
// FIXME - all of 'em.  I think the latter is probably easier to deal with.
// FIXME - Provide a DEFAULT() macro for config.
// FIXME - 
// typedef struct ftseg_device_handle_s {
//     ht16k33_device_handle_t *ht16k33;
//     uint8_t disp_mask;
//     // char text[HT16K33_NDEVS][HT16K33_DIGITS_PER_DEV + 1];
// } ftseg_device_handle_t;



// typedef enum {
//     ANIM_SEQ_VERT_SCROLL,
//     ANIM_SEQ_TYPE_HORZ_SCROLL,
//     ANIM_SEQ_BEAM_IN
//     // use the vert-scroll advanced across all the digits, each three-frame sequence
//     // ends with a digit 'beaming' in, moving from left to right.

// } anim_seq_t;

typedef enum {
    ANIM_FLAG_REVERSE = 0b001,    // Runs sequence in opposite direction
    ANIM_FLAG_SEQUENTIAL = 0b010, // Changes characters one at a time
    ANIM_FLAG_ADD_BLANK = 0b100,  // Adds a blank frame to the end
} anim_flags_t;

typedef enum {
    FTSEG_ANIM_NONE = 0,
    FTSEG_ANIM_SCAN_START,
    FTSEG_ANIM_SCAN_ACTIVE,
    FTSEG_ANIM_SCAN_SUCCESS,
} ftseg_anim_t;


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


void ftseg_init(void);

/* Sets the ftseg text buffer. */
void ftseg_set_text(char *text);
void ftseg_test(void);
void ftseg_enable_decimal_point(bool enable);

/* 14-segment Animations */
void ftseg_anim_start(ftseg_anim_t which, uint16_t period_ms);
void ftseg_anim_update(void);
bool ftseg_anim_is_done(void);
// ESP_EVENT_DECLARE_BASE(FTSEG_EVENTS);
// enum {
//     FTSEG_EVENT_WRITE_TEXT,
//     FTSEG_EVENT_WRITE_SEGMENTS,
//     FTSEG_EVENT_SET_BRIGHTNESS,

// };


#endif // __FTSEG_H__