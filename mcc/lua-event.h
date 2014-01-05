#ifndef MWRT_LUA_EVENT_H
#define MWRT_LUA_EVENT_H

#include <lua.h>
#include <crt/event.h>

typedef void (*lua_signal_marshal_fn)(struct lua_State *L, va_list ap);

int lua_signal_new(struct lua_State *L,
                   struct signal *,
                   lua_signal_marshal_fn mfn,
                   int objref);

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
