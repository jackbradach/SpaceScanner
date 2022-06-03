#ifndef __FX_H__
#define __FX_H__

typedef enum {
    // FX_NONE,
    FX_STATIC,
    FX_TRIANGLE
    // FX_STARTUP
} fx_sound_t;

void fx_init(void);
void fx_play(fx_sound_t sound, bool loop);
void fx_calc_next_sample(void);
void fx_stop(void);
bool fx_is_done(void);
void fx_test(void);

#endif // __FX_H__