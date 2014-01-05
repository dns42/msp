#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <mcc/lua.h>
#include <mcc/lua-rcvec.h>

#include <crt/log.h>
#include <crt/list.h>

#include <alloca.h>
#include <assert.h>
#include <ctype.h>

struct lua_RCVec *
lua_rcvec_check(struct lua_State *L, int n)
{
    struct lua_RCVec *V;

    V = luaL_checkudata(L, n, "RCVec");
    luaL_argcheck(L, V != NULL, n, "`RCVec' expected");

    return V;
}

static int
lua_rcvec_checkindex(struct lua_State *L,
                     struct lua_RCVec *V, int idx)
{
    int n;

    n = -1;

    switch (lua_type(L, idx)) {
    case LUA_TSTRING: {
        const char *s;
        char c;

        s = lua_tostring(L, idx);
        c = tolower(s[0]);

        switch (c) {
        case 'r':
            n = 0;
            break;
        case 'p':
            n = 1;
            break;
        case 'y':
            n = 2;
            break;
        case 't':
            n = 3;
            break;
        default:
            if ('0' <= c && c <= '9') {
                n = c - '0';
                break;
            }
        }
        break;
    }
    case LUA_TNUMBER:
        n = lua_tonumber(L, idx);
        break;
    }

    luaL_argcheck(L, 0 <= n && n < V->len, 2,
                  "valid index expected");

    return n;
}

static int
lua_rcvec_index(struct lua_State *L)
{
    struct lua_RCVec *V;
    int n;

    V = lua_rcvec_check(L, 1);
    n = lua_rcvec_checkindex(L, V, 2);

    lua_pushnumber(L, V->val[n]);

    return 1;
}

static int
lua_rcvec_newindex(struct lua_State *L)
{
    struct lua_RCVec *V;
    int n, v;

    V = lua_rcvec_check(L, 1);
    n = lua_rcvec_checkindex(L, V, 2);
    v = luaL_checknumber(L, 3);

    luaL_argcheck(L, !v || (1000 <= v && v <= 2000), 3,
                  "number in range {0, 1000 .. 2000} expected");

    V->val[n] = v;

    return 1;
}

static int
lua_rcvec_new(struct lua_State *L)
{
    struct lua_RCVec *V;
    size_t size;
    int len;

    len = lua_gettop(L)
        ? luaL_checknumber(L, 1)
        : 8;

    size = offsetof(struct lua_RCVec, val[len]);

    V = lua_newuserdata(L, size);
    luaL_getmetatable(L, "RCVec");
    lua_setmetatable(L, -2);

    memset(V, 0, size);

    V->len = len;

    if (lua_gettop(L) > 2) {
        lua_getglobal(L, "RCVec");
        lua_getfield(L, -1, "update");
        lua_remove(L, -2);
        lua_pushvalue(L, -2);
        lua_pushvalue(L, 2);
        lua_call(L, 2, 0);
    }

    return 1;
}

static int
lua_rcvec_len(struct lua_State *L)
{
    struct lua_RCVec *V;

    V = lua_rcvec_check(L, 1);

    lua_pushnumber(L, V->len);

    return 1;
}

static int
lua_rcvec_tostring(struct lua_State *L)
{
    struct lua_RCVec *V;
    size_t max;
    char *str, *pos;
    int i, end;

    V = lua_rcvec_check(L, 1);

    max  = sizeof("[]");
    max += V->len * (sizeof("65535") - 1);
    max += V->len ? (V->len - 1) * (sizeof(", ") - 1) : 0;

    pos = str = alloca(max);
    pos[0] = '[';
    pos++;
    end = V->len - 1;

    for (i = 0; i < V->len; i++)
        pos += sprintf(pos, "%d%s",
                       V->val[i],
                       i < end ? ", " : "");

    pos += sprintf(pos, "]");

    lua_pushlstring(L, str, pos - str);

    return 1;
}

static const struct luaL_reg lua_rcvec_fn [] = {
    { "new", lua_rcvec_new },
    { NULL, NULL }
};

static const struct luaL_reg lua_rcvec_class [] = {
    { NULL, NULL }
};

static const struct luaL_reg lua_rcvec_meta [] = {
    { "__len", lua_rcvec_len },
    { "__index", lua_rcvec_index },
    { "__newindex", lua_rcvec_newindex },
    { "__tostring", lua_rcvec_tostring },
    { NULL, NULL }
};

int
luaopen_rcvec(struct lua_State *L)
{
    int rc;

    rc = luaL_loadbuffer(L,
                         MCC_LUA_START(rcvec), MCC_LUA_LEN(rcvec),
                         "rcvec.lua");
    assert(!rc);
    lua_call(L, 0, 0);

    luaL_openlib(L, "RCVec", lua_rcvec_fn, 0);
    lua_pop(L, 1);

    lua_object_classinit(L, "RCVec",
                         lua_rcvec_class,
                         lua_rcvec_meta);

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
