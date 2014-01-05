#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <mcc/lua.h>
#include <mcc/mcc.h>

extern struct mcc *g_mcc;

static int
__mcc_stop(lua_State *L)
{
    mcc_stop(g_mcc);
    return 0;
}

static const struct luaL_reg mcc_f [] = {
    { "stop", __mcc_stop },
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
