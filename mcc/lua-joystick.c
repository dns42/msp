#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <mcc/lua.h>
#include <mcc/lua-event.h>
#include <mcc/js.h>

#include <crt/log.h>
#include <crt/list.h>
#include <crt/event.h>

#include <assert.h>
#include <sys/types.h>

struct lua_Joystick {
    struct js *c;
    struct list events;
};

struct lua_JoystickEvent {
    struct signal *c;

    struct lua_State *L;
    struct lua_Joystick *J;

    enum js_ctl_type type;
    int idx;

    int fnr; /** registry callback ref */
    int evr; /** registry event ref */
};

static struct lua_Joystick *
lua_joystick_check(struct lua_State *L, int n)
{
    struct lua_Joystick *J;

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
        lua_pushinteger(L, val);
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

static void
lua_joystick_ctl_marshal(struct lua_State *L, va_list ap)
{
    lua_pushinteger(L, va_arg(ap, enum js_ctl_type));
    lua_pushinteger(L, va_arg(ap, int));
    lua_pushinteger(L, va_arg(ap, int));
}

static int
lua_joystick_link(struct lua_State *L)
{
    struct lua_Joystick *J;
    struct signal *sig;
    int jsref;

    sig = NULL;

    J = lua_joystick_check(L, 1);

    if (J->c) {
        const char *change;

        if (lua_isnumber(L, 2)) {
            enum js_ctl_type type;
            int idx;

            type = luaL_checknumber(L, 2);
            idx = luaL_checknumber(L, 3);
            change = luaL_checkstring(L, 4);

            sig = js_ctl_link(J->c, type, idx, change);
        } else {
            change = lua_tostring(L, 2);

            sig = js_link(J->c, change);
        }
    }

    if (sig) {
        lua_settop(L, 1);
        jsref = luaL_ref(L, LUA_REGISTRYINDEX);
    }

    return sig
        ? lua_signal_new(L, sig, lua_joystick_ctl_marshal, jsref)
        : 0;
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

    J = lua_joystick_check(L, 1);

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
    { "link", lua_joystick_link },
    { "close", lua_joystick_close },
    { "plugged", lua_joystick_plugged },
};

static const struct luaL_reg lua_joystick_meta [] = {
    { "__tostring", lua_joystick_tostring },
    { "__gc", lua_joystick_close },
    { NULL, NULL }
};

int
luaopen_joystick(struct lua_State *L)
{
    int rc;

    rc = luaL_loadbuffer(L,
                         MCC_LUA_START(joystick), MCC_LUA_LEN(joystick),
                         "joystick.lua");
    assert(!rc);
    lua_call(L, 0, 0);

    luaL_openlib(L, "Joystick", lua_joystick_fn, 0);
    lua_pop(L, 1);

    lua_object_classinit(L, "Joystick",
                         lua_joystick_class,
                         lua_joystick_meta);

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
