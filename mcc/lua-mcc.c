#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <mcc/lua.h>
#include <mcc/mcc.h>

extern struct mcc *g_mcc;

static int
lua_mcc_stop(lua_State *L)
{
    int status;
    const char *reason;

    status = luaL_checkinteger(L, 1);
    reason = luaL_checkstring(L, 2);

    mcc_stop(g_mcc, status, reason);

    return 0;
}

static const struct luaL_reg mcc_f [] = {
    { "stop", lua_mcc_stop },
    { NULL, NULL }
};

int
luaopen_mcc(lua_State *L)
{
    luaL_openlib(L, "mcc", mcc_f, 0);
    lua_pop(L, 1);

    return 0;
}

/*
 * Local variables:
 * mode: C
 * c-file-style: "Linux"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
