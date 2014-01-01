#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <mcc/lua.h>

#include <assert.h>

struct lua_RMI {
};

int
luaopen_rmi(struct lua_State *L)
{
    int rc;
    rc = luaL_loadbuffer(L,
                         MCC_LUA_START(rmi), MCC_LUA_LEN(rmi),
                         "rmi.lua");
    assert(!rc);
    lua_call(L, 0, 0);

    return 0;
}

MCC_LUA_INIT(luaopen_rmi);

/*
 * Local variables:
 * mode: C
 * c-file-style: "Linux"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
