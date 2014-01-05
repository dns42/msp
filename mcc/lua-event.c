#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <mcc/lua.h>
#include <mcc/lua-event.h>

#include <assert.h>

struct lua_Signal {
    struct lua_State *L;
    struct signal *sig;
    lua_signal_marshal_fn mfn;
    int funref;
    int sigref;
    int objref;
};

int
lua_signal_new(struct lua_State *L, struct signal *sig,
               lua_signal_marshal_fn mfn, int objref)
{
    struct lua_Signal *S;

    S = lua_newuserdata(L, sizeof(*S));

    luaL_getmetatable(L, "Signal");
    lua_setmetatable(L, -2);

    S->L = L;
    S->sig = sig;
    S->mfn = mfn;
    S->funref = -1;
    S->sigref = -1;
    S->objref = objref;

    return 1;
}

static struct lua_Signal *
lua_signal_check(struct lua_State *L, int n)
{
    struct lua_Signal *S;

    S = luaL_checkudata(L, n, "Signal");
    luaL_argcheck(L, S != NULL, n, "`Signal' expected");

    return S;
}

static int
lua_signal_disconnect(struct lua_State *L)
{
    struct lua_Signal *S;

    S = lua_signal_check(L, 1);

    signal_disconnect(S->sig);

    if (S->funref != -1) {
        luaL_unref(L, LUA_REGISTRYINDEX, S->funref);
        S->funref = -1;
    }

    if (S->sigref != -1) {
        luaL_unref(L, LUA_REGISTRYINDEX, S->sigref);
        S->sigref = -1;
    }

    return 0;
}

static int
lua_signal_destroy(struct lua_State *L)
{
    struct lua_Signal *S;

    S = lua_signal_check(L, 1);

    lua_signal_disconnect(L);

    if (S->objref != -1) {
        luaL_unref(L, LUA_REGISTRYINDEX, S->objref);
        S->objref = -1;
    }

    signal_destroy(S->sig);

    return 0;
}

static void
lua_signal_handle(void *data, va_list ap)
{
    struct lua_Signal *S;
    struct lua_State *L;
    int n;

    S = data;
    L = S->L;

    n = lua_gettop(L);

    lua_rawgeti(L, LUA_REGISTRYINDEX, S->funref);

    if (S->objref != -1)
        lua_rawgeti(L, LUA_REGISTRYINDEX, S->objref);

    S->mfn(L, ap);

    n = lua_gettop(L) - n - 1;

    lua_call(L, n, 0);
}

static int
lua_signal_connect(struct lua_State *L)
{
    struct lua_Signal *S;

    S = lua_signal_check(L, 1);

    if (S->funref != -1)
        lua_signal_disconnect(L);

    S->funref = luaL_ref(L, LUA_REGISTRYINDEX);

    lua_settop(L, 1);

    S->sigref = luaL_ref(L, LUA_REGISTRYINDEX);

    signal_connect(S->sig, lua_signal_handle, S);

    return 0;
}

static int
lua_signal_tostring(struct lua_State *L)
{
    struct lua_Signal *S;

    S = lua_signal_check(L, 1);

    lua_pushfstring(L, "signal %p", S);

    return 1;
}

static const struct luaL_reg lua_signal_class [] = {
    { "connect", lua_signal_connect },
    { "disconnect", lua_signal_disconnect },
    { NULL, NULL }
};

static const struct luaL_reg lua_signal_meta [] = {
    { "__tostring", lua_signal_tostring },
    { "__gc", lua_signal_destroy },
    { NULL, NULL }
};

int
luaopen_event(struct lua_State *L)
{
    int rc;

    rc = luaL_loadbuffer(L,
                         MCC_LUA_START(event), MCC_LUA_LEN(event),
                         "event.lua");
    assert(!rc);
    lua_call(L, 0, 0);

    lua_object_classinit(L, "Signal",
                         lua_signal_class,
                         lua_signal_meta);

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
