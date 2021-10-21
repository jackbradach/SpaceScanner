#ifndef _DBG_H_
#define _DBG_H_

#include "config.h"

void dbg_init(void);

#if defined(CONFIG_DBG_HILO_ENABLE)
inline void dbg_hi(void) { CONFIG_DBG_HILO_PORT |= _BV(CONFIG_DBG_HILO_BIT); }
inline void dbg_lo(void) { CONFIG_DBG_HILO_PORT &= ~_BV(CONFIG_DBG_HILO_BIT); }
#endif

#endif // _DBG_H_
