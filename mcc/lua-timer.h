#ifndef MWRT_LUA_TIMER_H
#define MWRT_LUA_TIMER_H

#include <sys/time.h>

lua_Number lua_timeton(const struct timeval *tv);

struct timeval lua_ntotime(lua_Number t);

struct timeval lua_checktime(struct lua_State *L, int n);

#endif

/*
 * Local variables:
 * mode: C
 * c-file-style: "Linux"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
