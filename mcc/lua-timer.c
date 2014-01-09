#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <mcc/lua.h>
#include <mcc/lua-timer.h>
#include <mcc/lua-event.h>

#include <crt/timer.h>

#include <assert.h>

struct lua_Timer {
    struct lua_State *L;
    struct timer *t;
    struct timeval ticktv;
    struct event *tick;
};

static const struct event_info lua_timer_event_tab[] = {
    EVENT_INFO(struct lua_Timer, tick),
    EVENT_INFO_NULL,
};

static struct lua_Timer *
lua_timer_check(struct lua_State *L, int n)
{
    struct lua_Timer *T;

    T = luaL_checkudata(L, n, "Timer");
    luaL_argcheck(L, T != NULL, n, "`Timer' expected");

    return T;
}

lua_Number
lua_timeton(const struct timeval *tv)
{
    lua_Number t;

    t  = 1.0 * tv->tv_sec;
    t += 1.0 * tv->tv_usec / 1000000;

    return t;
}

struct timeval
lua_ntotime(lua_Number t)
{
    struct timeval tv;

    tv.tv_sec = t;
    tv.tv_usec = ((t - tv.tv_sec) * 1000000);

    return tv;
}

struct timeval
lua_checktime(struct lua_State *L, int n)
{
    return lua_ntotime(luaL_checknumber(L, n));
}

static int
lua_timer_start(struct lua_State *L)
{
    struct lua_Timer *T;
    struct timeval t;

    T = lua_timer_check(L, 1);
    t = lua_checktime(L, 2);

    timer_interval(T->t, &t);

    return 0;
}

static int
lua_timer_restart(struct lua_State *L)
{
    struct lua_Timer *T;
    struct timeval t;

    T = lua_timer_check(L, 1);
    t = lua_checktime(L, 2);

    if (timerisset(&T->ticktv))
        timer_restart(T->t, &T->ticktv, &t);
    else
        timer_interval(T->t, &t);

    return 0;
}

static int
lua_timer_stop(struct lua_State *L)
{
    struct lua_Timer *T;

    T = lua_timer_check(L, 1);

    timer_stop(T->t);

    return 0;
}

static int
lua_timer_link(struct lua_State *L)
{
    struct lua_Timer *T;
    const char *change;
    struct signal *sig;
    int objref;

    T = lua_timer_check(L, 1);
    change = luaL_checkstring(L, 2);

    sig = event_lookup(T, change, lua_timer_event_tab);

    lua_settop(L, 1);
    objref = luaL_ref(L, LUA_REGISTRYINDEX);

    return lua_signal_new(L, sig, NULL, objref);
}

static void
lua_timer_fn(const struct timeval *timeo, void *data)
{
    struct lua_Timer *T;
    struct lua_State *L;

    T = data;
    L = T->L;

    T->ticktv = *timeo;

    event_emit(T->tick);

    timerclear(&T->ticktv);

    (void) L;
}

static int
lua_timer_new(struct lua_State *L)
{
    struct lua_Timer *T;

    T = lua_newuserdata(L, sizeof(*T));

    luaL_getmetatable(L, "Timer");
    lua_setmetatable(L, -2);

    memset(T, 0, sizeof(*T));

    T->L = L;
    T->t = evtloop_create_timer(g_loop, lua_timer_fn, T);

    return T->t ? 1 : 0;
}

static int
lua_timer_destroy(struct lua_State *L)
{
    struct lua_Timer *T;

    T = lua_timer_check(L, 1);

    timer_destroy(T->t);

    return 0;
}

static int
lua_timer_tostring(struct lua_State *L)
{
    struct lua_Timer *T;

    T = lua_timer_check(L, 1);

    lua_pushfstring(L, "timer %p", T);

    return 1;
}

static const struct luaL_reg lua_signal_class [] = {
    { "new", lua_timer_new },
    { "start", lua_timer_start },
    { "restart", lua_timer_restart },
    { "stop", lua_timer_stop },
    { "link", lua_timer_link },
    { NULL, NULL }
};

static const struct luaL_reg lua_signal_meta [] = {
    { "__tostring", lua_timer_tostring },
    { "__gc", lua_timer_destroy },
    { NULL, NULL }
};

int
luaopen_timer(struct lua_State *L)
{
    int rc;

    rc = luaL_loadbuffer(L,
                         MCC_LUA_START(timer), MCC_LUA_LEN(timer),
                         "timer.lua");
    assert(!rc);
    lua_call(L, 0, 0);

    lua_object_initclass(L, "Timer",
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
