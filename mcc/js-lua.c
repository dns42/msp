#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <mcc/lua.h>
#include <mcc/js.h>

#include <crt/log.h>
#include <crt/list.h>

#include <assert.h>
#include <sys/types.h>

struct lua_Joystick {
    struct js *c;
    struct list events;
};

struct lua_JoystickEvent {
    struct lua_State *L;
    struct lua_Joystick *J;

    struct list entry;
    enum js_ctl_type type;
    int idx;

    int jsr; /** registry joystick ref */
    int fnr; /** registry callback ref */
    int evr; /** registry event ref */
};

static struct lua_Joystick *
lua_joystick_check(struct lua_State *L, int n)
{
    struct lua_Joystick *J;

    lua_getmetatable(L, -1);
    lua_pop(L, 1);

    J = luaL_checkudata(L, n, "Joystick");
    luaL_argcheck(L, J != NULL, n, "`Joystick' expected");

    return J;
}

static int
lua_joystick_name(struct lua_State *L)
{
    struct lua_Joystick *J;
    int rc;

    J = lua_joystick_check(L, 1);

    rc = expected(J->c) ? 0 : -1;
    if (!rc) {
        const char *name;

        name = js_name(J->c);

        lua_pushstring(L, name);
    } else
        lua_pushnil(L);

    return 1;
}

static int
lua_joystick_value(struct lua_State *L, enum js_ctl_type type)
{
    struct lua_Joystick *J;
    lua_Number idx;
    int val;

    J = lua_joystick_check(L, 1);
    idx = luaL_checknumber(L, 2);

    val = JS_VAL_NIL;

    if (expected(J->c))
        val = js_value(J->c, type, (int) idx);

    if (val != JS_VAL_NIL)
        lua_pushnumber(L, val);
    else
        lua_pushnil(L);

    return 1;
}

static int
lua_joystick_axis(struct lua_State *L)
{
    return lua_joystick_value(L, JS_AXIS);
}

static int
lua_joystick_button(struct lua_State *L)
{
    return lua_joystick_value(L, JS_BUTTON);
}

static struct lua_JoystickEvent *
lua_joystick_event_check(struct lua_State *L, int n)
{
    void *ud;

    ud = luaL_checkudata(L, n, "JoystickEvent");
    luaL_argcheck(L, ud != NULL, 1, "`JoystickEvent' expected");

    return ud;
}

static void
__lua_joystick_event_disconnect(struct lua_JoystickEvent *evt)
{
    struct lua_State *L;

    L = evt->L;

    if (evt->type) {
        js_evt_disconnect(evt->J->c, evt->type, evt->idx);
        evt->type = 0;
    }

    if (evt->fnr >= 0) {
        luaL_unref(L, LUA_REGISTRYINDEX, evt->fnr);
        evt->fnr = -1;
    }

    if (evt->evr >= 0) {
        luaL_unref(L, LUA_REGISTRYINDEX, evt->evr);
        evt->evr = -1;
    }
}

static int
lua_joystick_event_disconnect(struct lua_State *L)
{
    struct lua_JoystickEvent *evt;

    evt = lua_joystick_event_check(L, 1);

    __lua_joystick_event_disconnect(evt);

    return 0;
}

static int
lua_joystick_event_tostring(struct lua_State *L)
{
    struct lua_JoystickEvent *evt;

    evt = lua_joystick_event_check(L, 1);

    lua_pushfstring(L, "JoystickEvent: %s, %d",
                    evt->type == JS_AXIS
                    ? "AXIS"
                    : evt->type == JS_BUTTON
                    ? "BUTTON"
                    : "?",
                    evt->idx);

    return 0;
}

static void
__lua_joystick_js_event(struct js *js,
                        enum js_ctl_type type, int idx, int val,
                        void *data)
{
    struct lua_JoystickEvent *evt;
    struct lua_State *L;

    evt = data;
    L = evt->L;

    lua_rawgeti(L, LUA_REGISTRYINDEX, evt->fnr);
    lua_rawgeti(L, LUA_REGISTRYINDEX, evt->jsr);
    lua_pushnumber(L, type);
    lua_pushnumber(L, idx);
    lua_pushnumber(L, val);
    lua_call(L, 4, 0);
}

static int
lua_joystick_connect(struct lua_State *L)
{
    struct lua_Joystick *J;
    enum js_ctl_type type;
    int idx;

    J = lua_joystick_check(L, 1);
    type = luaL_checknumber(L, 2);
    idx = luaL_checknumber(L, 3);
    luaL_checktype(L, 4, LUA_TFUNCTION);

    if (expected(J->c)) {
        struct lua_JoystickEvent *evt;
        int rc;

        evt = lua_newuserdata(L, sizeof(*evt));
        evt->L = L;
        evt->J = J;
        evt->fnr = -1;
        evt->evr = -1;

        luaL_getmetatable(L, "JoystickEvent");
        lua_setmetatable(L, -2);

        evt->evr = luaL_ref(L, LUA_REGISTRYINDEX);
        evt->fnr = luaL_ref(L, LUA_REGISTRYINDEX);
        lua_settop(L, 1);
        evt->jsr = luaL_ref(L, LUA_REGISTRYINDEX);

        rc = js_evt_connect(J->c,
                            type, idx,
                            __lua_joystick_js_event, evt);
        if (!rc) {
            evt->type = type;
            evt->idx = idx;
        }

        list_insert_tail(&J->events, &evt->entry);

        return 1;
    }

    return 0;
}

static int
lua_joystick_tostring(struct lua_State *L)
{
    struct lua_Joystick *J;

    J = lua_joystick_check(L, 1);

    if (J->c) {
        struct stat st;

        js_stat(J->c, &st);

        lua_pushfstring(L, "Joystick ('%s', dev %d:%d)",
                        js_name(J->c),
                        major(st.st_rdev), minor(st.st_rdev));
    } else
        lua_pushstring(L, "Joystick (closed)");

    return 1;
}

static int
lua_joystick_new(struct lua_State *L, struct js *js)
{
    struct lua_Joystick *J;

    J = lua_newuserdata(L, sizeof(*J));
    luaL_getmetatable(L, "Joystick");
    lua_setmetatable(L, -2);

    J->c = js;
    list_init(&J->events);

    js_plug(js, g_loop);

    return 1;
}

static int
lua_joystick_open(struct lua_State *L)
{
    const char *path;
    struct js *js;

    path = luaL_checkstring(L, 1);

    js = js_open(path);

    return js ? lua_joystick_new(L, js) : 0;
}

static int
lua_joystick_close(struct lua_State *L)
{
    struct lua_Joystick *J;
    struct lua_JoystickEvent *evt, *next;

    J = lua_joystick_check(L, 1);

    list_for_each_entry_safe(&J->events, evt, next, entry)
        __lua_joystick_event_disconnect(evt);

    if (J->c) {
        js_close(J->c);
        J->c = NULL;
    }

    return 0;
}

static int
lua_joystick_plugged(struct lua_State *L)
{
    struct lua_Joystick *J;

    J = lua_joystick_check(L, 1);

    lua_pushboolean(L, J->c && js_plugged(J->c));

    return 1;
}

static const struct luaL_reg lua_joystick_fn [] = {
    { "open", lua_joystick_open },
    { NULL, NULL }
};

static const struct luaL_reg lua_joystick_class[] = {
    { "name", lua_joystick_name },
    { "axis", lua_joystick_axis },
    { "button", lua_joystick_button },
    { "connect", lua_joystick_connect },
    { "close", lua_joystick_close },
    { "plugged", lua_joystick_plugged },
};

static const struct luaL_reg lua_joystick_meta [] = {
    { "__tostring", lua_joystick_tostring },
    { "__gc", lua_joystick_close },
    { NULL, NULL }
};

static const struct luaL_reg lua_joystick_event_class [] = {
    { "disconnect", lua_joystick_event_disconnect },
};

static const struct luaL_reg lua_joystick_event_meta [] = {
    { "__tostring", lua_joystick_event_tostring },
    { NULL, NULL }
};

int
luaopen_js(struct lua_State *L)
{
    int rc;

    rc = luaL_loadbuffer(L,
                         MCC_LUA_START(js), MCC_LUA_LEN(js),
                         "js.lua");
    assert(!rc);
    lua_call(L, 0, 0);

    luaL_openlib(L, "Joystick", lua_joystick_fn, 0);
    lua_pop(L, 1);

    lua_object_classinit(L, "Joystick",
                         lua_joystick_class,
                         lua_joystick_meta);

    lua_object_classinit(L, "JoystickEvent",
                         lua_joystick_event_class,
                         lua_joystick_meta);

    return 0;
}

MCC_LUA_INIT(luaopen_js);

/*
 * Local variables:
 * mode: C
 * c-file-style: "Linux"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
