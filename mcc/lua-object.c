#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <mcc/lua.h>

#include <crt/log.h>

#include <stdlib.h>
#include <assert.h>

void
lua_object_getclass(struct lua_State *L,
                    const char *_name)
{
    char *name, *pos, *key;

    name = strdup(_name);
    assert(name);

    key = pos = name;
    key = strsep(&pos, ".");

    lua_getglobal(L, key);
    assert(lua_istable(L, -1));

    while (pos) {
        key = strsep(&pos, ".");
        lua_getfield(L, -1, key);
        assert(lua_istable(L, -1));
        lua_remove(L, -2);
    }

    free(name);
}

void
lua_object_initclass(struct lua_State *L,
                     const char *name,
                     const struct luaL_reg *reg_class,
                     const struct luaL_reg *reg_meta)
{
    info("init class %s..", name);

    lua_object_getclass(L, name);

    if (reg_class)
        luaL_openlib(L, NULL, reg_class, 0);

    lua_getmetatable(L, -1);
    lua_getfield(L, -1, "__objmt");
    lua_remove(L, -2);

    if (reg_meta)
        luaL_openlib(L, NULL, reg_meta, 0);

    lua_setfield(L, LUA_REGISTRYINDEX, name);
    lua_pop(L, 1);
}

int
luaopen_object(struct lua_State *L)
{
    int rc;

    rc = luaL_loadbuffer(L,
                         MCC_LUA_START(object),
                         MCC_LUA_LEN(object),
                         "object.lua");
    assert(!rc);

    lua_call(L, 0, 0);

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