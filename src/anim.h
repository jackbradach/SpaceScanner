#ifndef __ANIM_H__
#define __ANIM_H__

#include "ftseg.h"
#include "fx.h"

typedef enum {
    ANIM_STARTUP,
    ANIM_SCAN_START,
    ANIM_SCAN_ACTIVE
} anim_t;

void anim_init(void);
void anim_start(anim_t which);
void anim_update(void);

#endif // __ANIM_H__